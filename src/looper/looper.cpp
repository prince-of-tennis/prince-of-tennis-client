#include "looper.hpp"

#include "core/context.hpp"
#include "opengl/opengl.hpp"
#include "util/log.hpp"
#include "window_manager/window_manager.hpp"

static Context g_context;
static OpenGL g_opengl;

bool looper_init(Looper *looper, eSceneType default_scene)
{
    // SDL初期化
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        LOG_ERROR("SDLの初期化に失敗しました: " << SDL_GetError());
        return false;
    }
    LOG_DEBUG("SDLを初期化しました");

    // ウィンドウサイズの設定
    g_context.window_width = 800;
    g_context.window_height = 600;

    // WindowManager初期化
    if (!window_manager_init(&g_context)) return false;

    g_context.window = get_window();
    SDL_GetWindowSize(g_context.window, &g_context.window_width, &g_context.window_height);

    // OpenGL初期化
    if (!opengl_init(&g_opengl, &g_context))
    {
        LOG_ERROR("OpenGLの初期化に失敗しました");
        return false;
    }
    LOG_DEBUG("OpenGLを初期化しました");

    // SceneManager初期化
    looper->scene_manager = make_unique<SceneManager>();
    scene_manager_init(looper->scene_manager, default_scene);

    return true;
}

void loop(Looper *looper)
{
    SDL_bool is_running = SDL_TRUE;
    while (is_running)
    {
        // イベント処理
        is_running = window_manager_update();

        if (is_running)
        {
            // シーン更新
            is_running = scene_update(looper->scene_manager);

            // バッファスワップ
            SDL_GL_SwapWindow(g_context.window);
        }
    }
}

void looper_fini(Looper *looper)
{
    scene_manager_fini(looper->scene_manager);
    window_manager_fini();
    SDL_Quit();
    LOG_DEBUG("SDLを終了しました");
}
