#pragma once

#include <SDL2/SDL.h>

#include <glm/glm.hpp>

struct Context
{
    SDL_Window *window;
    int window_width;
    int window_height;
    glm::vec3 camera_position;
    glm::vec3 camera_target;
    float background_r;
    float background_g;
    float background_b;
};