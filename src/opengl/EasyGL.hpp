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

/// @brief 背景色の設定と画面クリア
/// @param r 赤成分 (0-255)
/// @param g 緑成分 (0-255)
/// @param b 青成分 (0-255)
/// @param a アルファ成分 (0-255, デフォルト255)
void EZ_BackgroundClear(int r, int g, int b, int a = 255);