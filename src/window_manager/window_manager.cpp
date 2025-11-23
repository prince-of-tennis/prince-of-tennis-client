#include "window_manager.hpp"

#include "util/log.hpp"

static WindowManager g_window_mgr;

bool window_manager_init()
{
    g_window_mgr.window.reset(SDL_CreateWindow("超次元テニス", SDL_WINDOWPOS_CENTERED,
                                               SDL_WINDOWPOS_CENTERED, 1920, 1080,
                                               SDL_WINDOW_SHOWN));

    if (g_window_mgr.window == nullptr)
    {
        LOG_ERROR("ウィンドウの生成に失敗しました: " << SDL_GetError());
        return false;
    }
    LOG_DEBUG("ウィンドウを生成しました");
    return true;
}

SDL_bool window_manager_update()
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

void window_manager_fini()
{
}

SDL_Window *get_window()
{
    return g_window_mgr.window.get();
}