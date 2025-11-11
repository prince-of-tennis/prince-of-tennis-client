#include "scene_manager.hpp"

#include "util/log.hpp"

static void init_scene(scene_type scene);
static void set_scene(scene_type scene);

static scene_type g_current_scene;

void init_scene_manager(scene_type default_scene)
{
    set_scene(default_scene);
}

static void init_scene(scene_type scene)
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

SDL_bool update_scene()
{
    // MARK: メイン処理
    // ここにそれぞれのシーンで毎回実行する処理を記述する
    switch (g_current_scene)
    {
        case SCENE_GAME:
            LOG_DEBUG("SCENE_GAME");
            break;

        case SCENE_TITLE:
            LOG_DEBUG("SCENE_TITLE");
            break;
    }

    return SDL_TRUE;
}

void change_scene(scene_type scene)
{
    // MARK: 終了処理
    // ここにそれぞれのシーンの終了処理を記述する
    switch (g_current_scene)
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

    set_scene(scene);
}

/// @brief シーンを設定する
/// @param scene シーン
static void set_scene(scene_type scene)
{
    init_scene(scene);
    g_current_scene = scene;
}