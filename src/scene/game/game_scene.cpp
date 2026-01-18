#include "game_scene.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glad/glad.h"
#include "network/network.hpp"
#include "opengl/2d/EZ_2d.h"
#include "opengl/EasyGL.hpp"
#include "game_scene_camera.hpp"
#include "game_scene_network.hpp"
#include "util/log.hpp"

// スコア取得ヘルパー関数
static int get_player_score(const GameScore &score, int player_id)
{
    return (player_id == 0) ? score.current_game_p1 : score.current_game_p2;
}

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
    for (int i = 0; i < PLAYER_MAX; i++)
    {
        scene->player_objects[i] = EZ_CreateObject("obj/player.obj", "img/container.jpeg");
        if (scene->player_objects[i] == nullptr)
        {
            LOG_ERROR("プレイヤーオブジェクト" << i << "の作成に失敗しました");
            return false;
        }

        // プレイヤーの初期座標を設定（PLAYER_INITIAL_X は player_id ごとに異なる）
        scene->player_data[i].point.x = PLAYER_INITIAL_X[i];
        scene->player_data[i].point.y = PLAYER_INITIAL_Y;
        scene->player_data[i].point.z = PLAYER_INITIAL_Z;
        EZ_ObjectSetPosition(scene->player_objects[i], scene->player_data[i].point.x,
                             scene->player_data[i].point.y, scene->player_data[i].point.z);
        EZ_ObjectSetScale(scene->player_objects[i], 1.0, 1.0, 1.0);
    }
    LOG_SUCCESS("プレイヤーオブジェクト初期化完了");

    // テニスコートオブジェクトの初期化
    scene->court_object = EZ_CreateObject("obj/tennis_court.obj", "img/container.jpeg");
    if (scene->court_object == nullptr)
    {
        LOG_ERROR("テニスコートオブジェクトの作成に失敗しました");
        return false;
    }
    EZ_ObjectSetPosition(scene->court_object, 0.0f, 0.0f, 0.0f);
    EZ_ObjectSetScale(scene->court_object, TENNIS_COURT_SCALE, TENNIS_COURT_SCALE,
                      TENNIS_COURT_SCALE);
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
    game_scene_init_camera_position(scene);

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
    game_scene_update_camera(scene);

    // プレイヤー入力をサーバーに送信
    game_scene_send_player_input(scene, player_input);

    // スイングデータをサーバーに送信
    game_scene_send_player_swing(scene, player_swing);

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

    const GameScore &game_score = scene->network->network_data_set.game_score;

    // ヘルパー関数を使用してスコアを取得
    int my_points = get_player_score(game_score, my_player_id);
    int opponent_points = get_player_score(game_score, opponent_id);

    // デバッグ: スコアデータを確認（60フレームごとに1回出力）
    static int debug_counter = 0;
    if (debug_counter % 60 == 0)
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
    float screen_width = static_cast<float>(scene->context->window_width);
    float x = screen_width / 2.0f - SCORE_OFFSET_X;  // 中央寄せ
    float y = SCORE_POS_Y;
    float size = SCORE_FONT_SIZE;

    // 色設定（白）
    float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;

    // スコアを描画
    EZ_2D_DrawText(scene->font, x, y, score_text, size, r, g, b, a);
}
