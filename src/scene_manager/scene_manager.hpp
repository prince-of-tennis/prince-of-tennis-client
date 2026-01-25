#pragma once

#include <SDL2/SDL.h>

#include <memory>

#include "common/player_input.h"
#include "common/player_swing.h"
#include "connection/connection_manager.hpp"
#include "core/context.hpp"
#include "joycon/joycon.hpp"
#include "network/network.hpp"
#include "scene/game/game_scene.hpp"
#include "scene/matching/matching_scene.hpp"
#include "scene/title/title_scene.hpp"
#include "scene_type.hpp"

using namespace std;

struct SceneManager
{
    eSceneType current_scene;
    Context *context;
    Joycon *joycon;
    TitleScene title_scene;
    MatchingScene matching_scene;
    GameScene game_scene;
    PlayerInput player_input;

    // SceneManager が所有する Network
    Network network;

    // 接続マネージャー（別スレッドでジョイコン接続を管理）
    ConnectionManager connection_manager;

    // 初期化フラグ
    bool joycon_initialized;
    bool network_initialized;
};

/// @brief scene_managerの初期化
/// @param default_scene デフォルトのシーン
bool scene_manager_init(unique_ptr<SceneManager> &mgr, eSceneType default_scene);

/// @brief scene_managerの終了処理
/// @param mgr SceneManager
void scene_manager_fini(unique_ptr<SceneManager> &mgr);

/// @brief シーンを指定した実行する
/// @return ループするか(SDL_FALSEの時に終了)
bool scene_update(unique_ptr<SceneManager> &mgr, PlayerInput *player_input,
                  PlayerSwing *player_swing);

/// @brief シーンを変更
/// @param scene シーン
bool scene_change(unique_ptr<SceneManager> &mgr, eSceneType scene);