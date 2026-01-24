#include "matching_scene.hpp"

#include "glad/glad.h"
#include "input/input_manager.hpp"
#include "util/log.hpp"

// テニスコートスケール（title_scene と同じ）
constexpr float TENNIS_COURT_SCALE = 0.2f;
// コート回転速度
constexpr float COURT_ROTATION_SPEED = 0.2f;
// フォントサイズ
constexpr float STATUS_FONT_SIZE = 36.0f;

// Joy-Con 接続フェーズの処理
static void update_joycon_phase(MatchingScene *scene)
{
    // 前回失敗してリトライ待機中
    if (scene->last_attempt_failed)
    {
        scene->retry_wait_frames--;
        if (scene->retry_wait_frames <= 0)
        {
            scene->last_attempt_failed = false;
            scene->retry_count++;
        }
        return;
    }

    // Joy-Con 初期化を試行
    scene->status_message = "Joy-Conに接続中";

    if (joycon_init(scene->joycon))
    {
        LOG_SUCCESS("マッチングシーン: Joy-Con 接続成功");
        if (scene->joycon_initialized_ptr)
        {
            *scene->joycon_initialized_ptr = true;
        }
        scene->phase = MATCHING_PHASE_SERVER;
        scene->last_attempt_failed = false;
        scene->retry_count = 0;
    }
    else
    {
        LOG_WARN("マッチングシーン: Joy-Con 接続失敗、リトライ待機");
        scene->status_message = "Joy-Conに接続中";
        scene->last_attempt_failed = true;
        scene->retry_wait_frames = RETRY_WAIT_FRAMES;
    }
}

// サーバー接続フェーズの処理
static void update_server_phase(MatchingScene *scene)
{
    // 前回失敗してリトライ待機中
    if (scene->last_attempt_failed)
    {
        scene->retry_wait_frames--;
        if (scene->retry_wait_frames <= 0)
        {
            scene->last_attempt_failed = false;
            scene->retry_count++;
        }
        return;
    }

    // サーバー接続を試行
    scene->status_message = "サーバーに接続中";

    if (network_init(scene->network, scene->context))
    {
        LOG_SUCCESS("マッチングシーン: サーバー接続成功、対戦相手を待機中");
        if (scene->network_initialized_ptr)
        {
            *scene->network_initialized_ptr = true;
        }
        scene->phase = MATCHING_PHASE_WAITING;
        scene->status_message = "対戦相手を待っています";
        scene->last_attempt_failed = false;
        scene->retry_count = 0;
    }
    else
    {
        LOG_WARN("マッチングシーン: サーバー接続失敗、リトライ待機");
        scene->status_message = "サーバに接続中";
        scene->last_attempt_failed = true;
        scene->retry_wait_frames = RETRY_WAIT_FRAMES;
    }
}

// マッチング待機フェーズの処理
static void update_waiting_phase(MatchingScene *scene)
{
    scene->status_message = "対戦相手を待っています";

    // サーバーからのデータを受信
    if (!network_listen_to_server(scene->network))
    {
        LOG_ERROR("マッチングシーン: サーバー通信エラー");
        return;
    }

    // player_id が設定されたらマッチング完了
    if (scene->context->player_id != -1)
    {
        LOG_SUCCESS("マッチングシーン: マッチング完了 player_id=" << scene->context->player_id);
        scene->phase = MATCHING_PHASE_COMPLETE;
        scene->complete_counter = COMPLETE_DISPLAY_FRAMES;

        // 決定SE再生
        if (scene->se_decide >= 0)
        {
            audio_play_se(&scene->audio, scene->se_decide);
        }
    }
}

// 完了フェーズの処理
static MatchingResult update_complete_phase(MatchingScene *scene)
{
    scene->status_message = "マッチング完了！";
    scene->complete_counter--;

    if (scene->complete_counter <= 0)
    {
        return MATCHING_RESULT_SUCCESS;
    }
    return MATCHING_RESULT_NONE;
}

bool matching_scene_init(MatchingScene *scene)
{
    // フォント読み込み
    scene->font = EZ_2D_CreateFont("fonts/font.otf", 48);
    if (!scene->font)
    {
        LOG_ERROR("マッチングシーン: フォントの読み込みに失敗しました");
        return false;
    }

    // 3D シェーダー初期化
    scene->shader = EZ_CreateShader();
    if (scene->shader == nullptr)
    {
        LOG_ERROR("マッチングシーン: シェーダーの作成に失敗しました");
        return false;
    }

    // カメラ初期化（俯瞰視点）
    float aspect_ratio = static_cast<float>(scene->context->window_width) /
                         static_cast<float>(scene->context->window_height);
    scene->camera = EZ_CreateCamera(aspect_ratio);
    EZ_CameraSetPosition(scene->camera, 0.0f, 15.0f, 25.0f);
    EZ_CameraSetTargetPosition(scene->camera, 0.0f, 0.0f, 0.0f);
    EZ_CameraSetClipPlanes(scene->camera, 0.1f, 200.0f);

    // ライト初期化
    scene->light = EZ_CreateLight();
    EZ_LightSetPosition(scene->light, 10.0f, 20.0f, 10.0f);

    // オーディオ初期化
    if (!audio_init(&scene->audio))
    {
        LOG_ERROR("マッチングシーン: オーディオの初期化に失敗しました");
        return false;
    }

    // SE 読み込み
    scene->se_decide = audio_load_se(&scene->audio, "audio/se/select.mp3");

    // 変数初期化
    scene->phase = MATCHING_PHASE_JOYCON;
    scene->retry_count = 0;
    scene->retry_wait_frames = 0;
    scene->last_attempt_failed = false;
    scene->status_message = "";
    scene->dot_animation_counter = 0;
    scene->complete_counter = 0;

    LOG_DEBUG("マッチングシーンを初期化しました");
    return true;
}

MatchingResult matching_scene_update(MatchingScene *scene)
{
    // ドットアニメーション更新
    scene->dot_animation_counter++;
    if (scene->dot_animation_counter >= DOT_ANIMATION_INTERVAL * 4)
    {
        scene->dot_animation_counter = 0;
    }

    // キャンセル処理（ESC キー）
    if (input_is_key_just_pressed(KEY_ESCAPE))
    {
        LOG_DEBUG("マッチングシーン: キャンセルされました");
        return MATCHING_RESULT_CANCEL;
    }

    // フェーズごとの処理
    switch (scene->phase)
    {
        case MATCHING_PHASE_JOYCON:
            update_joycon_phase(scene);
            break;

        case MATCHING_PHASE_SERVER:
            update_server_phase(scene);
            break;

        case MATCHING_PHASE_WAITING:
            update_waiting_phase(scene);
            break;

        case MATCHING_PHASE_COMPLETE:
            return update_complete_phase(scene);
    }

    return MATCHING_RESULT_NONE;
}

void matching_scene_draw(MatchingScene *scene)
{
    // 背景をセピア調（タイトルと同じ）でクリア
    glClearColor(0.85f, 0.78f, 0.65f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 2D 描画
    float screen_width = static_cast<float>(scene->context->window_width);
    float screen_height = static_cast<float>(scene->context->window_height);

    // ステータスメッセージ描画
    if (scene->status_message && scene->status_message[0] != '\0')
    {
        // ドットアニメーション（「...」を 0〜3 個で表示）
        int dot_count = scene->dot_animation_counter / DOT_ANIMATION_INTERVAL;
        char message_with_dots[256];
        snprintf(message_with_dots, sizeof(message_with_dots), "%s%.*s", scene->status_message,
                 dot_count, "...");

        // 画面中央に表示
        float x = screen_width / 2.0f - 200.0f;
        float y = screen_height / 2.0f;

        EZ_2D_DrawText(scene->font, x, y, message_with_dots, STATUS_FONT_SIZE, 0.3f, 0.2f, 0.1f,
                       1.0f);
    }
}

void matching_scene_fini(MatchingScene *scene)
{
    // フォント解放
    scene->font.reset();

    // オーディオ終了
    audio_fini(&scene->audio);

    LOG_DEBUG("マッチングシーンを終了しました");
}
