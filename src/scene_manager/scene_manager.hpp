#pragma once

#include <SDL2/SDL.h>

#include <memory>

#include "scene_type.hpp"

using namespace std;

struct SceneManager
{
    eSceneType current_scene;
};

/// @brief scene_managerの初期化
/// @param default_scene デフォルトのシーン
void init_scene_manager(unique_ptr<SceneManager> &mgr, eSceneType default_scene);

/// @brief scene_managerの終了処理
/// @param mgr SceneManager
void fini_scene_manager(unique_ptr<SceneManager> &mgr);

/// @brief シーンを指定した実行する
/// @return ループするか(SDL_FALSEの時に終了)
SDL_bool update_scene(unique_ptr<SceneManager> &mgr);

/// @brief シーンを変更
/// @param scene シーン
void change_scene(unique_ptr<SceneManager> &mgr, eSceneType scene);