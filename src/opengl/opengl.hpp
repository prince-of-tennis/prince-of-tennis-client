#pragma once

#include <SDL2/SDL.h>

#include <vector>

#include "core/context.hpp"
#include "model/opengl_model.hpp"
#include "shader/opengl_shader.hpp"

using namespace std;

struct OpenGL
{
    SDL_GLContext context;
};

/// @brief OpenGL関連の初期化
/// @param gl OpenGL
/// @param context Context
/// @return 成功したか
bool opengl_init(OpenGL *gl, Context *context);