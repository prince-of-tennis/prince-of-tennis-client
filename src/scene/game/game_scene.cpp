#include "game_scene.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glad/glad.h"
#include "network/network.hpp"
#include "opengl/2d/EZ_2d.h"
#include "opengl/EasyGL.hpp"
#include "util/log.hpp"

bool game_scene_init(GameScene *scene)
{
    LOG_DEBUG("GameScene初期化開始");

    // シェーダー初期化
    scene->shader = EZ_CreateShader();
    if (scene->shader == nullptr)
    {
        LOG_ERROR("シェーダーの作成に失敗しました");
        return false;
    }
    LOG_SUCCESS("シェーダー初期化完了");

    // ボールオブジェクトの初期化
    scene->ball_object = EZ_CreateObject("obj/ball.obj", "img/container.jpeg");
    if (scene->ball_object == nullptr)
    {
        LOG_ERROR("ボールオブジェクトの作成に失敗しました");
        return false;
    }

    // ボールの初期座標を設定（テスト用）
    scene->ball_data.point.x = 0.0f;
    scene->ball_data.point.y = 2.0f;
    scene->ball_data.point.z = 0.0f;

    EZ_ObjectSetPosition(scene->ball_object, scene->ball_data.point.x, scene->ball_data.point.y,
                         scene->ball_data.point.z);
    EZ_ObjectSetScale(scene->ball_object, 0.5, 0.5, 0.5);

    LOG_SUCCESS("ボールオブジェクト初期化完了");

    // プレイヤーオブジェクトの初期化
    for (int i = 0; i < 2; i++)
    {
        scene->player_objects[i] = EZ_CreateObject("obj/player.obj", "img/container.jpeg");
        if (scene->player_objects[i] == nullptr)
        {
            LOG_ERROR("プレイヤーオブジェクト" << i << "の作成に失敗しました");
            return false;
        }
    }

    // プレイヤーの初期座標を設定（テスト用）
    // プレイヤー1（左側）
    scene->player_data[0].point.x = 0.0f;
    scene->player_data[0].point.y = 0.0f;
    scene->player_data[0].point.z = 0.0f;
    EZ_ObjectSetPosition(scene->player_objects[0], scene->player_data[0].point.x,
                         scene->player_data[0].point.y, scene->player_data[0].point.z);
    EZ_ObjectSetScale(scene->player_objects[0], 1.0, 1.0, 1.0);

    // プレイヤー2（右側）
    scene->player_data[1].point.x = 10.0f;
    scene->player_data[1].point.y = 0.0f;
    scene->player_data[1].point.z = 0.0f;
    EZ_ObjectSetPosition(scene->player_objects[1], scene->player_data[1].point.x,
                         scene->player_data[1].point.y, scene->player_data[1].point.z);
    EZ_ObjectSetScale(scene->player_objects[1], 1.0, 1.0, 1.0);

    LOG_SUCCESS("プレイヤーオブジェクト初期化完了");

    // テニスコートオブジェクトの初期化
    scene->court_object = EZ_CreateObject("obj/tennis_court.obj", "img/container.jpeg");
    if (scene->court_object == nullptr)
    {
        LOG_ERROR("テニスコートオブジェクトの作成に失敗しました");
        return false;
    }
    EZ_ObjectSetPosition(scene->court_object, 0.0f, 0.0f, 0.0f);
    EZ_ObjectSetScale(scene->court_object, 0.511242f, 0.511242f, 0.511242f);
    LOG_SUCCESS("テニスコートオブジェクト初期化完了");

    // グラウンドオブジェクトの初期化
    scene->ground_object = EZ_CreateObject("obj/ground.obj", "img/container.jpeg");
    if (scene->ground_object == nullptr)
    {
        LOG_ERROR("グラウンドオブジェクトの作成に失敗しました");
        return false;
    }
    EZ_ObjectSetPosition(scene->ground_object, 0.0f, 0.0f, 0.0f);
    EZ_ObjectSetScale(scene->ground_object, 1.0f, 1.0f, 1.0f);
    LOG_SUCCESS("グラウンドオブジェクト初期化完了");

    // ライト初期化
    scene->light = EZ_CreateLight();

    // Network
    scene->network.reset(new Network);
    if (!network_init(scene->network.get(), scene->context))
    {
        return false;
    }

    // サーバーからプレイヤーIDを受信（ゲーム開始時）
    LOG_DEBUG("プレイヤーID受信待機中...");
    const Uint32 TIMEOUT_MS = 5000;  // 5秒タイムアウト

    while (true)
    {
        if (!network_listen_to_server(scene->network.get()))
        {
            LOG_ERROR("プレイヤーID受信中にエラーが発生しました");
            return false;
        }

        // プレイヤーIDが設定されたか確認
        if (scene->context->player_id != -1)
        {
            LOG_SUCCESS("プレイヤーID受信完了: " << scene->context->player_id);
            break;
        }

        SDL_Delay(10);  // CPU負荷軽減
    }

    // カメラ初期化（player_id受信後）
    scene->camera = EZ_CreateCamera(static_cast<float>(scene->context->window_width) /
                                    static_cast<float>(scene->context->window_height));

    // プレイヤーIDに応じてカメラ位置を設定
    // player_id == 0: コート奥側から見る（Z正方向）
    // player_id == 1: コート手前側から見る（Z負方向）
    float camera_z = (scene->context->player_id == 0) ? CAMERA_POS_Z : -CAMERA_POS_Z;

    LOG_DEBUG("カメラ設定: player_id=" << scene->context->player_id << " camera_z=" << camera_z);

    EZ_CameraSetPosition(scene->camera, CAMERA_POS_X, CAMERA_POS_Y, camera_z);
    EZ_CameraSetTargetPosition(scene->camera, CAMERA_TARGET_X, CAMERA_TARGET_Y, CAMERA_TARGET_Z);

    // カメラの描画範囲を拡大（Z軸方向にオブジェクトが見えるように）
    EZ_CameraSetClipPlanes(scene->camera, 0.1f, 200.0f);

    // サーバー同期カウンターを初期化
    scene->server_sync_counter = 0;

    // フォントの初期化
    scene->font = EZ_2D_CreateFont("fonts/font.otf", 48);
    if (scene->font == nullptr)
    {
        LOG_ERROR("フォントの作成に失敗しました");
        return false;
    }
    LOG_SUCCESS("フォント初期化完了");

    // スコアの初期化
    memset(&scene->game_score, 0, sizeof(GameScore));

    // 前フレームの入力を初期化
    LOG_SUCCESS("GameScene初期化完了");
    return true;
}

bool game_scene_update(GameScene *scene, PlayerInput *player_input, PlayerSwing *player_swing)
{
    if (!network_listen_to_server(scene->network.get()))
    {
        return false;
    }

    // ネットワークからボールのデータを取得して反映
    scene->ball_data = scene->network->network_data_set.ball;

    // ボールオブジェクトの座標を更新
    EZ_ObjectSetPosition(scene->ball_object, scene->ball_data.point.x, scene->ball_data.point.y,
                         scene->ball_data.point.z);

    // スコアデータの更新
    scene->game_score = scene->network->network_data_set.game_score;

    // サーバー同期カウンターをインクリメント
    scene->server_sync_counter++;
    bool should_sync_with_server = (scene->server_sync_counter >= SERVER_SYNC_INTERVAL);
    if (should_sync_with_server)
    {
        scene->server_sync_counter = 0;
    }

    for (int i = 0; i < PLAYER_MAX; i++)
    {
        scene->player_data[i] = scene->network->network_data_set.players[i];
        EZ_ObjectSetPosition(scene->player_objects[i], scene->player_data[i].point.x,
                             scene->player_data[i].point.y, scene->player_data[i].point.z);
    }

    // カメラ追従処理（自分のプレイヤーに追従）
    int my_player_id = scene->context->player_id;
    if (my_player_id >= 0 && my_player_id < PLAYER_MAX)
    {
        Player &my_player = scene->player_data[my_player_id];

        // カメラ位置: プレイヤーの後方に配置
        float camera_z = (my_player_id == 0) ? CAMERA_POS_Z : -CAMERA_POS_Z;
        float camera_x = my_player.point.x + CAMERA_POS_X;
        float camera_y = CAMERA_POS_Y;
        float camera_z_offset = my_player.point.z + camera_z;

        EZ_CameraSetPosition(scene->camera, camera_x, camera_y, camera_z_offset);

        // 注視点: プレイヤーの位置を見る（オフセットは加算しない）
        float target_x = my_player.point.x;
        float target_y = CAMERA_TARGET_Y;
        float target_z = my_player.point.z;

        EZ_CameraSetTargetPosition(scene->camera, target_x, target_y, target_z);
    }

    // Joy-Conのデータが変化した場合のみサーバに送信
    if (player_input != nullptr)
    {
        PlayerInput adjusted_input = *player_input;

        // player_id == 1の場合は、カメラが反対側なので入力を反転
        if (scene->context->player_id == 1)
        {
            bool temp_front = adjusted_input.front;
            adjusted_input.front = adjusted_input.back;
            adjusted_input.back = temp_front;

            bool temp_left = adjusted_input.left;
            adjusted_input.left = adjusted_input.right;
            adjusted_input.right = temp_left;
        }

        Packet joycon_packet;
        joycon_packet.type = PACKET_TYPE_PLAYER_INPUT;
        joycon_packet.size = sizeof(PlayerInput);
        memcpy(joycon_packet.data, &adjusted_input, sizeof(PlayerInput));
        network_send_to_server(scene->network.get(), &joycon_packet);
    }

    // スイングデータが閾値を超えた場合にサーバに送信
    if (player_swing != nullptr)
    {
        PlayerSwing adjusted_swing = *player_swing;

        // player_id == 0の場合
        if (scene->context->player_id == 0)
        {
            // z軸を反転（Joy-Conの座標系とゲーム座標系の違いを補正）
            adjusted_swing.acc_z = -adjusted_swing.acc_z;
        }
        // player_id == 1の場合
        else if (scene->context->player_id == 1)
        {
            // カメラが反対側なので、x軸のみ反転（z軸はそのまま）
            adjusted_swing.acc_x = -adjusted_swing.acc_x;
        }

        Packet swing_packet;
        swing_packet.type = PACKET_TYPE_PLAYER_SWING;
        swing_packet.size = sizeof(PlayerSwing);
        memcpy(swing_packet.data, &adjusted_swing, sizeof(PlayerSwing));
        network_send_to_server(scene->network.get(), &swing_packet);
    }

    return true;
}

void game_scene_draw(GameScene *scene)
{
    // 画面クリア
    EZ_BackgroundClear(scene->context->background_r, scene->context->background_g,
                       scene->context->background_b);

    // グラウンドの描画
    if (scene->ground_object)
    {
        EZ_DrawObject(scene->ground_object, scene->shader, scene->camera, scene->light);
    }

    // テニスコートの描画
    if (scene->court_object)
    {
        EZ_DrawObject(scene->court_object, scene->shader, scene->camera, scene->light);
    }

    // ボールの描画
    if (scene->ball_object)
    {
        EZ_DrawObject(scene->ball_object, scene->shader, scene->camera, scene->light);
    }

    // プレイヤーの描画
    for (int i = 0; i < PLAYER_MAX; i++)
    {
        if (scene->player_objects[i])
        {
            EZ_DrawObject(scene->player_objects[i], scene->shader, scene->camera, scene->light);
        }
    }

    // スコア表示
    draw_score(scene);
}

// スコア表示関数
void draw_score(GameScene *scene)
{
    int my_player_id = scene->context->player_id;
    int opponent_id = (my_player_id == 0) ? 1 : 0;

    NetworkDataSet data_set = scene->network->network_data_set;

    // 自分のポイント
    int my_points = (my_player_id == 0) ? data_set.game_score.current_game_p1
                                        : data_set.game_score.current_game_p2;

    // 相手のポイント
    int opponent_points = (opponent_id == 0) ? data_set.game_score.current_game_p1
                                             : data_set.game_score.current_game_p2;

    // デバッグ: スコアデータを確認
    static int debug_counter = 0;
    if (debug_counter % 60 == 0)  // 60フレームごとに1回出力
    {
        LOG_DEBUG("スコア表示 - 自分:" << my_points << " 相手:" << opponent_points
                                       << " player_id:" << my_player_id
                                       << " font:" << (scene->font != nullptr ? "OK" : "NULL"));
    }
    debug_counter++;

    // フォントがない場合は描画しない
    if (scene->font == nullptr)
    {
        return;
    }

    // スコア文字列を作成（例: "15 : 30"）
    char score_text[64];
    snprintf(score_text, sizeof(score_text), "%d : %d", my_points, opponent_points);

    // 画面中央上部に表示
    float screen_width = scene->context->window_width;
    float x = screen_width / 2.0f - 100.0f;  // 中央寄せ
    float y = 50.0f;
    float size = 60.0f;

    // 色設定（白）
    float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;

    // スコアを描画
    EZ_2D_DrawText(scene->font, x, y, score_text, size, r, g, b, a);
}
