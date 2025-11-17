#include "window_manager.hpp"

#include "util/log.hpp"

void window_manager_init(unique_ptr<WindowManager> &mgr)
{
    mgr->window = SDL_CreateWindow("超次元テニス", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                   1920, 1080, SDL_WINDOW_SHOWN);

    if (mgr->window == nullptr)
    {
        LOG_ERROR("ウィンドウの生成に失敗しました: " << SDL_GetError());
    }
    else
    {
        LOG_DEBUG("ウィンドウを生成しました");
    }
}

SDL_bool window_manager_update(unique_ptr<WindowManager> &mgr)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            LOG_DEBUG("終了イベントを受信しました");
            return SDL_FALSE;
        }
        // TODO: EventManagerに処理を委譲
    }
    return SDL_TRUE;
}

void window_manager_fini(unique_ptr<WindowManager> &mgr)
{
    if (mgr->window != nullptr)
    {
        SDL_DestroyWindow(mgr->window);
        mgr->window = nullptr;
        LOG_DEBUG("ウィンドウを破棄しました");
    }
}