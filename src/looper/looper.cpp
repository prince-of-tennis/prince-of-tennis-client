#include "looper.hpp"

#include "util/log.hpp"

void looper_init(Looper *looper, eSceneType default_scene)
{
    // SDL初期化
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        LOG_ERROR("SDLの初期化に失敗しました: " << SDL_GetError());
        return;
    }
    LOG_DEBUG("SDLを初期化しました");

    // WindowManager初期化
    looper->window_manager = make_unique<WindowManager>();
    window_manager_init(looper->window_manager);

    // SceneManager初期化
    looper->scene_manager = make_unique<SceneManager>();
    scene_manager_init(looper->scene_manager, default_scene);
}

void loop(Looper *looper)
{
    SDL_bool is_running = SDL_TRUE;
    while (is_running)
    {
        // イベント処理
        is_running = window_manager_update(looper->window_manager);

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
    window_manager_fini(looper->window_manager);
    SDL_Quit();
    LOG_DEBUG("SDLを終了しました");
}