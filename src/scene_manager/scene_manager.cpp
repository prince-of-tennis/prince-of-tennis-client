#include "scene_manager.hpp"

#include "util/log.hpp"

static void init_scene(unique_ptr<SceneManager> &mgr, eSceneType scene);
static void fini_scene(unique_ptr<SceneManager> &mgr);
static void set_scene(unique_ptr<SceneManager> &mgr, eSceneType scene);

void init_scene_manager(unique_ptr<SceneManager> &mgr, eSceneType default_scene)
{
    set_scene(mgr, default_scene);
}

static void init_scene(unique_ptr<SceneManager> &mgr, eSceneType scene)
{
    // MARK: 初期化
    // ここにそれぞれのシーンの初期化処理を記述する
    switch (scene)
    {
        case SCENE_TITLE:
            LOG_DEBUG("SCENE_TITLEを初期化します。");
            break;

        case SCENE_GAME:
            LOG_DEBUG("SCENE_GAMEを初期化します。");
            break;

        default:
            LOG_ERROR("不正なシーンが渡されました。");
            break;
    }
}

SDL_bool update_scene(unique_ptr<SceneManager> &mgr)
{
    // MARK: メイン処理
    // ここにそれぞれのシーンで毎回実行する処理を記述する
    switch (mgr->current_scene)
    {
        case SCENE_TITLE:
            LOG_DEBUG("SCENE_TITLE");
            change_scene(mgr, SCENE_GAME);
            return SDL_TRUE;

        case SCENE_GAME:
            LOG_DEBUG("SCENE_GAME");
            return SDL_FALSE;

        default:
            LOG_ERROR("不正なシーンが渡されています");
            return SDL_FALSE;
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
            break;
        case SCENE_GAME:
            LOG_DEBUG("SCENE_GAMEを終了します。");
            break;

        default:
            LOG_ERROR("不正なシーンが渡されました。");
            break;
    }
}

void fini_scene_manager(unique_ptr<SceneManager> &mgr)
{
    fini_scene(mgr);
}

void change_scene(unique_ptr<SceneManager> &mgr, eSceneType scene)
{
    fini_scene(mgr);
    set_scene(mgr, scene);
}

/// @brief シーンを設定する
/// @param scene シーン
static void set_scene(unique_ptr<SceneManager> &mgr, eSceneType scene)
{
    init_scene(mgr, scene);
    mgr->current_scene = scene;
}