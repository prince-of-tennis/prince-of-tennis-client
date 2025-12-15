#include "looper.hpp"

#include "common/player_input.h"
#include "core/context.hpp"
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
    g_context.background_r = 0.2f;
    g_context.background_g = 0.3f;
    g_context.background_b = 0.3f;

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

    // SceneManager初期化
    looper->scene_manager = make_unique<SceneManager>();
    looper->scene_manager->context = &g_context;
    if (!scene_manager_init(looper->scene_manager, default_scene))
    {
        LOG_ERROR("シーンの初期化に失敗しました");
        return false;
    }

    // if (!joycon_init(&looper->joycon)) return false;

    return true;
}

void loop(Looper *looper)
{
    SDL_bool is_running = SDL_TRUE;

    while (is_running)
    {
        fps_frame_start();

        // イベント処理
        is_running = window_manager_update();

        if (is_running)
        {
            // シーン更新
            is_running = scene_update(looper->scene_manager);

            // バッファスワップ
            SDL_GL_SwapWindow(g_context.window);
        }

        fps_frame_end();

        // PlayerInput player_input = get_joycon(&looper->joycon);
        // if (player_input.right)
        // {
        //     LOG_DEBUG("右");
        // }
        // if (player_input.left)
        // {
        //     LOG_DEBUG("左");
        // }
        // if (player_input.front)
        // {
        //     LOG_DEBUG("後");
        // }
        // if (player_input.back)
        // {
        //     LOG_DEBUG("前");
        // }
        // if (player_input.swing)
        // {
        //     LOG_DEBUG("振る");
        // }
    }
}

void looper_fini(Looper *looper)
{
    scene_manager_fini(looper->scene_manager);
    window_manager_fini();
    // joycon_fini(&looper->joycon);
    SDL_Quit();
    LOG_SUCCESS("SDLを終了しました");
}
