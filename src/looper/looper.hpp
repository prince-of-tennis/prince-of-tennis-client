#pragma once

#include <SDL2/SDL.h>

#include <memory>

#include "scene_manager/scene_manager.hpp"

using namespace std;

struct Looper
{
    unique_ptr<SceneManager> scene_manager;
};

/// @brief looperの初期化
void looper_init(Looper *looper, eSceneType default_scene);

/// @brief ループ処理
void loop(Looper *looper);

/// @brief looperの終了処理
void looper_fini(Looper *looper);