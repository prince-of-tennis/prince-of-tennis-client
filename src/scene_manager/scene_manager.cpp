#include "scene_manager.hpp"

#include "util/log.hpp"

static bool init_scene(unique_ptr<SceneManager> &mgr);
static void fini_scene(unique_ptr<SceneManager> &mgr);
static bool set_scene(unique_ptr<SceneManager> &mgr, eSceneType scene);

bool scene_manager_init(unique_ptr<SceneManager> &mgr, eSceneType default_scene)
{
    return set_scene(mgr, default_scene);
}

static bool init_scene(unique_ptr<SceneManager> &mgr)
{
    // MARK: 初期化
    // ここにそれぞれのシーンの初期化処理を記述する
    switch (mgr->current_scene)
    {
        case SCENE_TITLE:
            LOG_DEBUG("SCENE_TITLEを初期化します。");
            mgr->title_scene.context = mgr->context;
            mgr->title_scene.joycon = mgr->joycon;
            return title_scene_init(&mgr->title_scene);

        case SCENE_GAME:
            LOG_DEBUG("SCENE_GAMEを初期化します。");

            mgr->game_scene.context = mgr->context;
            return game_scene_init(&mgr->game_scene);

        default:
            LOG_ERROR("不正なシーンが渡されました。");
            return false;
    }
}

bool scene_update(unique_ptr<SceneManager> &mgr, PlayerInput *player_input,
                  PlayerSwing *player_swing)
{
    // MARK: メイン処理
    // ここにそれぞれのシーンで毎回実行する処理を記述する
    switch (mgr->current_scene)
    {
        case SCENE_TITLE:
        {
            bool should_change = title_scene_update(&mgr->title_scene);
            title_scene_draw(&mgr->title_scene);
            if (should_change)
            {
                return scene_change(mgr, SCENE_GAME);
            }
            return true;
        }

        case SCENE_GAME:
            if (!game_scene_update(&mgr->game_scene, player_input, player_swing))
            {
                return false;
            }
            game_scene_draw(&mgr->game_scene);
            return true;

        default:
            LOG_ERROR("不正なシーンが渡されています");
            return false;
    }
}

static void fini_scene(unique_ptr<SceneManager> &mgr)
{
    // MARK: 終了処理
    // ここにそれぞれのシーンの終了処理を記述する
    switch (mgr->current_scene)
    {
        case SCENE_TITLE:
            LOG_DEBUG("SCENE_TITLEを終了します。");
            title_scene_fini(&mgr->title_scene);
            break;
        case SCENE_GAME:
            LOG_DEBUG("SCENE_GAMEを終了します。");
            break;

        default:
            LOG_ERROR("不正なシーンが渡されました。");
            break;
    }
}

void scene_manager_fini(unique_ptr<SceneManager> &mgr)
{
    fini_scene(mgr);
}

bool scene_change(unique_ptr<SceneManager> &mgr, eSceneType scene)
{
    fini_scene(mgr);
    return set_scene(mgr, scene);
}

/// @brief シーンを設定する
/// @param scene シーン
static bool set_scene(unique_ptr<SceneManager> &mgr, eSceneType scene)
{
    mgr->current_scene = scene;
    return init_scene(mgr);
}