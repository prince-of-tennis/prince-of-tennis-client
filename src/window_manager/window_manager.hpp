#pragma once

#include <SDL2/SDL.h>

#include <memory>

using namespace std;

struct WindowManager
{
    SDL_Window *window;
};

void window_manager_init(unique_ptr<WindowManager> &window_manager);
SDL_bool window_manager_update(unique_ptr<WindowManager> &window_manager);
void window_manager_fini(unique_ptr<WindowManager> &window_manager);