#include "window_manager.hpp"

#include "input/input_manager.hpp"
#include "util/log.hpp"

static WindowManager g_window_mgr;

bool window_manager_init(Context *context)
{
    g_window_mgr.window.reset(SDL_CreateWindow(
        "超次元テニス", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, context->window_width,
        context->window_height, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL));

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
    // 前フレームの入力状態を保存
    input_manager_update();

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            LOG_DEBUG("終了イベントを受信しました");
            return SDL_FALSE;
        }

        // キーボードイベントを InputManager に渡す
        input_manager_handle_event(event);
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