#pragma once

#include <SDL2/SDL.h>

#include <memory>

#include "core/context.hpp"
#include "scene_manager/scene_manager.hpp"
#include "window_manager/window_manager.hpp"

#include "joycon/joycon.hpp"

using namespace std;

struct Looper
{
    unique_ptr<SceneManager> scene_manager;
    Context *context;
    Joycon joycon;
};

/// @brief looperの初期化
bool looper_init(Looper *looper, eSceneType default_scene);

/// @brief ループ処理
void loop(Looper *looper);

/// @brief looperの終了処理
void looper_fini(Looper *looper);
