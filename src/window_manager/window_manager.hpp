#pragma once

#include <SDL2/SDL.h>

#include "util/helper.hpp"

using namespace std;

struct WindowManager
{
    UniquePtr<SDL_Window> window;
};

bool window_manager_init();
SDL_bool window_manager_update();
void window_manager_fini();

/// @brief SDL_Windowのアドレスを取得する
/// @return SDL_Window&
SDL_Window *get_window();