#include "looper.hpp"

#include "common/player_input.h"
#include "core/context.hpp"
#include "input/input_manager.hpp"
#include "joycon/joycon.hpp"
#include "opengl/2d/EZ_2d.h"
#include "opengl/EasyGL.hpp"
#include "util/log.hpp"
#include "window_manager/window_manager.hpp"

static Context g_context;
static EasyGL g_opengl;

// FPS制御用の変数
static const int TARGET_FPS = 60;
static const Uint32 TARGET_FRAME_TIME = 1000 / TARGET_FPS;  // ミリ秒

// FPS制御用の内部変数
static Uint32 g_frame_start = 0;

#ifdef DEBUG
// 将来的にFPS計測機能を実装する際に使用予定の変数
static Uint32 g_fps_timer = 0;
#endif

void context_init()
{
    g_context.window_width = 860;
    g_context.window_height = 640;
    g_context.camera_position = glm::vec3(0.0f, 30.0f, 90.0f);
    g_context.camera_target = glm::vec3(0.0f, 0.0f, 0.0f);
    g_context.background_r = 51;
    g_context.background_g = 76;
    g_context.background_b = 76;

    g_context.network_host = "localhost";
    g_context.network_port = 5000;
}

/// @brief フレーム開始時の処理
static void fps_frame_start()
{
    g_frame_start = SDL_GetTicks();
}

/// @brief フレーム終了時の処理（FPS制限と計測）
static void fps_frame_end()
{
    // フレーム時間を計算
    Uint32 frame_time = SDL_GetTicks() - g_frame_start;

    // 目標フレーム時間に満たない場合は待機
    if (frame_time < TARGET_FRAME_TIME)
    {
        SDL_Delay(TARGET_FRAME_TIME - frame_time);
    }
}

bool looper_init(Looper *looper, eSceneType default_scene)
{
    // SDL初期化
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        LOG_ERROR("SDLの初期化に失敗しました: " << SDL_GetError());
        return false;
    }
    LOG_SUCCESS("SDLを初期化しました");

    context_init();

    // WindowManager初期化
    if (!window_manager_init(&g_context)) return false;

    g_context.window = get_window();

    // OpenGL初期化
    if (!EZ_Init(&g_opengl, &g_context))
    {
        LOG_ERROR("OpenGLの初期化に失敗しました");
        return false;
    }
    LOG_SUCCESS("OpenGLを初期化しました");

    // 2D描画システム初期化
    if (!EZ_2D_Init(g_context.window_width, g_context.window_height))
    {
        LOG_ERROR("2D描画システムの初期化に失敗しました");
        return false;
    }

    // InputManager初期化
    input_manager_init();

    // Joy-Con はマッチングシーンで初期化されるため、ここでは初期化しない
    // Joycon 構造体だけ初期化
    memset(&looper->joycon, 0, sizeof(Joycon));

    // SceneManager初期化
    looper->scene_manager = make_unique<SceneManager>();
    looper->scene_manager->context = &g_context;
    looper->scene_manager->joycon = &looper->joycon;
    looper->scene_manager->joycon_initialized = false;
    looper->scene_manager->network_initialized = false;
    if (!scene_manager_init(looper->scene_manager, default_scene))
    {
        LOG_ERROR("シーンの初期化に失敗しました");
        return false;
    }

    return true;
}

void loop(Looper *looper)
{
    bool is_running = true;

    while (is_running)
    {
        fps_frame_start();

        PlayerInput player_input = {};
        PlayerSwing player_swing = {};
        bool has_movement_input = false;
        bool should_send_swing = false;

        // Joy-Con が初期化されている場合のみ入力を取得
        if (looper->scene_manager->joycon_initialized)
        {
            player_input = get_joycon(&looper->joycon, g_context.player_id);

            // ボタン状態を更新
            joycon_update_buttons(&looper->joycon);

            // 移動入力があるかチェック（どれか一つでもtrueなら入力あり）
            has_movement_input =
                player_input.left || player_input.right || player_input.front || player_input.back;

            // スイングデータを取得
            player_swing = get_joycon_swing(&looper->joycon, g_context.player_id);

            // スイング動作が閾値を超えたかチェック
            should_send_swing = joycon_has_significant_swing(&looper->joycon, &player_swing);
        }

        // イベント処理
        is_running = window_manager_update();

        if (is_running)
        {
            // シーン更新（移動入力があるかswing送信が必要な場合）
            if (has_movement_input || should_send_swing)
            {
                is_running = scene_update(looper->scene_manager,
                                          has_movement_input ? &player_input : nullptr,
                                          should_send_swing ? &player_swing : nullptr);

                // swingキャッシュを更新（連続送信を防ぐため）
                if (should_send_swing)
                {
                    looper->joycon.cached_swing = player_swing;
                }
            }
            else
            {
                // 入力がない場合はNULLを渡す
                is_running = scene_update(looper->scene_manager, nullptr, nullptr);
            }

            // バッファスワップ
            SDL_GL_SwapWindow(g_context.window);
        }

        fps_frame_end();
    }
}

void looper_fini(Looper *looper)
{
    scene_manager_fini(looper->scene_manager);

    // Joy-Con が初期化されている場合のみ終了処理
    bool joycon_was_initialized = looper->scene_manager->joycon_initialized;

    // SDL_Quit()の前にSceneManagerを破棄する
    // （GameSceneのEZ_2D_Font, EZ_2D_ImageなどがOpenGLコンテキスト有効時に解放されるように）
    looper->scene_manager.reset();

    if (joycon_was_initialized)
    {
        joycon_fini(&looper->joycon);
    }

    window_manager_fini();
    SDL_Quit();
    LOG_SUCCESS("SDLを終了しました");
}
