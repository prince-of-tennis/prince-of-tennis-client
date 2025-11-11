#pragma once

#include <SDL2/SDL.h>

#include "scene_type.hpp"

/// @brief scene_managerの初期化
/// @param default_scene デフォルトのシーン
void init_scene_manager(scene_type default_scene);

/// @brief シーンを指定した実行する
/// @return ループするか(SDL_FALSEの時に終了)
SDL_bool update_scene();

/// @brief シーンを変更
/// @param scene シーン
void change_scene(scene_type scene);