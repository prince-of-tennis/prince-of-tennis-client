#pragma once

#include <SDL2/SDL.h>

#include <vector>

#include "core/context.hpp"
#include "model/EZ_Model.hpp"
#include "shader/EZ_Shader.hpp"

using namespace std;

struct EasyGL
{
    SDL_GLContext context;

    ~EasyGL();
};

/// @brief OpenGL関連の初期化
/// @param gl OpenGL
/// @param context Context
/// @return 成功したか
bool EZ_Init(EasyGL *gl, Context *context);
