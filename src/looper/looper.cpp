#include "looper.hpp"

#include "core/context.hpp"
#include "util/log.hpp"

static Context g_context;

bool looper_init(Looper *looper, eSceneType default_scene)
{
    // SDL初期化
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        LOG_ERROR("SDLの初期化に失敗しました: " << SDL_GetError());
        return;
    }
    LOG_DEBUG("SDLを初期化しました");

    // WindowManager初期化
    if (!window_manager_init()) return false;

    g_context.window = get_window();
    SDL_GetWindowSize(g_context.window, &g_context.window_width, &g_context.window_height);

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
