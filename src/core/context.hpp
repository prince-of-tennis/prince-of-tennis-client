#pragma once

#include <SDL2/SDL.h>

#include <glm/glm.hpp>
#include <string>

using namespace std;

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

    // Network関連
    string network_host;
    Uint16 network_port;
    int player_id = -1;  // プレイヤーID（未設定は-1）
};