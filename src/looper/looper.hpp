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
void init_looper(Looper *looper, eSceneType default_scene);

/// @brief ループ処理
/// @return ループするか
void loop(Looper *looper);

/// @brief looperの終了処理
void finalize_looper(Looper *looper);