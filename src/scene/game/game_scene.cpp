#include "game_scene.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glad/glad.h"
#include "joycon/joycon.hpp"
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

bool game_scene_init(GameScene *scene, Network *network)
{
    LOG_DEBUG("GameScene初期化開始");

    // Network は SceneManager から渡される（既にマッチング済み）
    scene->network.reset(network);

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
    scene->ground_object = EZ_CreateObject("obj/ground.obj", "img/ground.png");
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

    // カメラ初期化（player_id は既にマッチングシーンで受信済み）
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
    memset(&scene->prev_game_score, 0, sizeof(GameScore));

    // オーディオ初期化
    if (!audio_init(&scene->audio))
    {
        LOG_ERROR("オーディオの初期化に失敗しました");
        return false;
    }

    // SE読み込み
    scene->se_hit_ball = audio_load_se(&scene->audio, "audio/se/hit_ball.mp3");
    scene->se_yatta = audio_load_se(&scene->audio, "audio/se/yatta.mp3");

    // ボール打撃検出用の初期化
    scene->prev_hit_count = 0;

    // 能力マネージャー初期化
    ability_manager_init(&scene->ability_manager);

    LOG_SUCCESS("GameScene初期化完了");
    return true;
}

void game_scene_fini(GameScene *scene)
{
    LOG_DEBUG("GameScene終了処理開始");

    // オーディオの終了処理
    audio_fini(&scene->audio);

    // networkはSceneManagerが所有しているため、unique_ptrから所有権を放棄する
    // （deleteしない）
    scene->network.release();

    LOG_DEBUG("GameScene終了処理完了");
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

    // ボール打撃検出（hit_countが増加したらSE再生）
    bool ball_hit_detected = (scene->ball_data.hit_count > scene->prev_hit_count);
    if (ball_hit_detected)
    {
        audio_play_se(&scene->audio, scene->se_hit_ball);
    }

    // スコアデータの更新
    scene->game_score = scene->network->network_data_set.game_score;

    // 自分が得点したときにSE再生
    int my_id = scene->context->player_id;
    int my_prev_score = (my_id == 0) ? scene->prev_game_score.current_game_p1
                                     : scene->prev_game_score.current_game_p2;
    int my_curr_score = (my_id == 0) ? scene->game_score.current_game_p1
                                     : scene->game_score.current_game_p2;
    if (my_curr_score > my_prev_score)
    {
        audio_play_se(&scene->audio, scene->se_yatta);
    }
    scene->prev_game_score = scene->game_score;

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

        // #86: でかすぎんだろ - サーバーから受信した能力状態でスケール変更
        const AbilityState& ability_state = scene->network->network_data_set.ability_states[i];
        bool is_giant = (ability_state.active_ability == ABILITY_GIANT && ability_state.remaining_frames > 0);
        if (is_giant)
        {
            EZ_ObjectSetScale(scene->player_objects[i], 10.0f, 10.0f, 10.0f);
        }
        else
        {
            EZ_ObjectSetScale(scene->player_objects[i], 1.0f, 1.0f, 1.0f);
        }
    }

    // カメラ追従処理（自分のプレイヤーに追従）
    game_scene_update_camera(scene);

    // プレイヤー入力をサーバーに送信
    game_scene_send_player_input(scene, player_input);

    // 能力ボタン状態更新（スイング送信前に更新）
    ability_manager_update(&scene->ability_manager, scene->joycon);

    // 即時発動能力のチェック
    AbilityActivateRequest* instant_req = ability_check_instant(&scene->ability_manager, my_id);
    if (instant_req)
    {
        game_scene_send_ability_request(scene, instant_req);
    }

    static bool prev_giant_button = false;
    static bool prev_clone_button = false;
    bool curr_giant_button = scene->ability_manager.button_held[ABILITY_GIANT];
    bool curr_clone_button = scene->ability_manager.button_held[ABILITY_CLONE];

    if (curr_giant_button != prev_giant_button)
    {
        AbilityActivateRequest req;
        req.player_id = my_id;
        req.ability_type = ABILITY_GIANT;
        req.trigger = curr_giant_button ? TRIGGER_INSTANT : TRIGGER_ON_HIT;
        game_scene_send_ability_request(scene, &req);
        prev_giant_button = curr_giant_button;
    }

    if (curr_clone_button != prev_clone_button)
    {
        LOG_DEBUG("Clone button changed: " << curr_clone_button << " -> sending " << (curr_clone_button ? "ACTIVATE" : "DEACTIVATE"));
        AbilityActivateRequest req;
        req.player_id = my_id;
        req.ability_type = ABILITY_CLONE;
        req.trigger = curr_clone_button ? TRIGGER_INSTANT : TRIGGER_ON_HIT;
        game_scene_send_ability_request(scene, &req);
        prev_clone_button = curr_clone_button;
    }

    // スイング時発動能力のチェック（スイングデータ送信前に能力リクエストを送信）
    if (player_swing != nullptr)
    {
        AbilityActivateRequest* swing_req = ability_check_on_swing(&scene->ability_manager, my_id);
        if (swing_req)
        {
            game_scene_send_ability_request(scene, swing_req);
        }
    }

    // スイングデータをサーバーに送信（能力リクエストの後に送信）
    game_scene_send_player_swing(scene, player_swing);

    // 打撃時発動能力のチェック（ボールを打った場合）
    if (ball_hit_detected)
    {
        AbilityActivateRequest* hit_req = ability_check_on_hit(&scene->ability_manager, my_id);
        if (hit_req)
        {
            game_scene_send_ability_request(scene, hit_req);
        }
    }

    // 能力フレームカウント更新
    ability_manager_tick(&scene->ability_manager);

    // 打撃カウントを更新（次フレーム用）
    scene->prev_hit_count = scene->ball_data.hit_count;

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

    // ボールの描画（#88: ABILITY_INVISIBLE_BALL がアクティブなら描画しない）
    bool ball_invisible = ability_is_local_active(&scene->ability_manager, ABILITY_INVISIBLE_BALL);
    if (ball_invisible)
    {
        // 発動中のフレーム数をログ出力（デバッグ用）
        uint32_t remaining = scene->ability_manager.local_states[ABILITY_INVISIBLE_BALL].remaining_frames;
        static uint32_t last_remaining = 0;
        if (remaining != last_remaining)
        {
            LOG_DEBUG("ボール消える発動中: 残り" << remaining << "frames");
            last_remaining = remaining;
        }
    }
    if (scene->ball_object && !ball_invisible)
    {
        EZ_DrawObject(scene->ball_object, scene->shader, scene->camera, scene->light);
    }

    // プレイヤーの描画
    constexpr float CLONE_OFFSET_X = 3.0f;
    for (int i = 0; i < PLAYER_MAX; i++)
    {
        if (!scene->player_objects[i])
        {
            continue;
        }

        EZ_DrawObject(scene->player_objects[i], scene->shader, scene->camera, scene->light);

        const AbilityState& ability_state = scene->network->network_data_set.ability_states[i];

        static int debug_counter = 0;
        if (debug_counter++ % 60 == 0)
        {
            LOG_DEBUG("Player " << i << " ability: " << static_cast<int>(ability_state.active_ability)
                      << " remaining: " << ability_state.remaining_frames);
        }

        if (ability_state.active_ability == ABILITY_CLONE && ability_state.remaining_frames > 0)
        {
            LOG_DEBUG("分身描画: player=" << i);
            float original_x = scene->player_data[i].point.x;

            EZ_ObjectSetPosition(scene->player_objects[i],
                                 original_x - CLONE_OFFSET_X,
                                 scene->player_data[i].point.y,
                                 scene->player_data[i].point.z);
            EZ_DrawObject(scene->player_objects[i], scene->shader, scene->camera, scene->light);

            EZ_ObjectSetPosition(scene->player_objects[i],
                                 original_x + CLONE_OFFSET_X,
                                 scene->player_data[i].point.y,
                                 scene->player_data[i].point.z);
            EZ_DrawObject(scene->player_objects[i], scene->shader, scene->camera, scene->light);

            EZ_ObjectSetPosition(scene->player_objects[i],
                                 original_x,
                                 scene->player_data[i].point.y,
                                 scene->player_data[i].point.z);
        }
    }

    // スコア表示
    draw_score(scene);
}

// スコア表示関数
// ポイントを表示用文字列に変換（50はAdvに変換）
static void points_to_string(int points, int opponent_points, char* buffer, size_t buffer_size)
{
    // デュース状態（両者40以上）で50ならAdv
    if (points == 50 && opponent_points >= 40)
    {
        snprintf(buffer, buffer_size, "Adv");
    }
    else
    {
        snprintf(buffer, buffer_size, "%d", points);
    }
}

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

    // ポイントスコア文字列を作成（50はAdvに変換）
    char my_score_str[8];
    char opp_score_str[8];
    points_to_string(my_points, opponent_points, my_score_str, sizeof(my_score_str));
    points_to_string(opponent_points, my_points, opp_score_str, sizeof(opp_score_str));
    char score_text[64];
    snprintf(score_text, sizeof(score_text), "%s : %s", my_score_str, opp_score_str);

    // 画面中央上部に表示（テキスト幅を考慮して中央揃え）
    float screen_width = static_cast<float>(scene->context->window_width);
    float y = SCORE_POS_Y;
    float size = SCORE_FONT_SIZE;

    // 色設定（白）
    float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;

    // テキスト幅を計算して中央揃え
    float text_width = EZ_2D_GetTextWidth(scene->font, score_text, size);
    float x = (screen_width - text_width) / 2.0f;

    // ポイントスコアを描画
    EZ_2D_DrawText(scene->font, x, y, score_text, size, r, g, b, a);

    // セット数（ゲーム数）を表示
    int current_set = game_score.current_set;
    int my_games = game_score.games_in_set[current_set][my_player_id];
    int opp_games = game_score.games_in_set[current_set][opponent_id];

    char set_text[64];
    snprintf(set_text, sizeof(set_text), "%d - %d", my_games, opp_games);

    // セット数もテキスト幅を計算して中央揃え
    float set_text_width = EZ_2D_GetTextWidth(scene->font, set_text, SET_SCORE_FONT_SIZE);
    float set_x = (screen_width - set_text_width) / 2.0f;
    float set_y = SCORE_POS_Y + SET_SCORE_OFFSET_Y;

    // セット数を小さめのフォントで描画
    EZ_2D_DrawText(scene->font, set_x, set_y, set_text, SET_SCORE_FONT_SIZE, r, g, b, a);
}
