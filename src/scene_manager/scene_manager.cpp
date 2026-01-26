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
            // ジョイコン接続済みならジョイコンを渡す
            mgr->title_scene.joycon = mgr->joycon_initialized ? mgr->joycon : nullptr;
            // ConnectionManagerへのポインタを渡す
            mgr->title_scene.connection_manager = &mgr->connection_manager;
            mgr->title_scene.joycon_initialized_ptr = &mgr->joycon_initialized;
            // タイトル画面で別スレッドでジョイコン接続を開始
            if (!mgr->joycon_initialized)
            {
                connection_manager_start_joycon_connect(&mgr->connection_manager);
            }
            return title_scene_init(&mgr->title_scene);

        case SCENE_MATCHING:
            LOG_DEBUG("SCENE_MATCHINGを初期化します。");
            mgr->matching_scene.context = mgr->context;
            mgr->matching_scene.joycon = mgr->joycon;
            mgr->matching_scene.network = &mgr->network;
            mgr->matching_scene.connection_manager = &mgr->connection_manager;
            mgr->matching_scene.joycon_initialized_ptr = &mgr->joycon_initialized;
            mgr->matching_scene.network_initialized_ptr = &mgr->network_initialized;
            return matching_scene_init(&mgr->matching_scene);

        case SCENE_GAME:
            LOG_DEBUG("SCENE_GAMEを初期化します。");
            mgr->game_scene.context = mgr->context;
            mgr->game_scene.joycon = mgr->joycon;
            mgr->game_scene.connection_manager = &mgr->connection_manager;
            // Network は SceneManager から渡す
            return game_scene_init(&mgr->game_scene, &mgr->network);

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
            TitleSceneResult result = title_scene_update(&mgr->title_scene);
            title_scene_draw(&mgr->title_scene);

            switch (result)
            {
                case TITLE_RESULT_START:
                    // マッチングシーンへ遷移
                    return scene_change(mgr, SCENE_MATCHING);
                case TITLE_RESULT_EXIT:
                    return false;
                default:
                    return true;
            }
        }

        case SCENE_MATCHING:
        {
            MatchingResult result = matching_scene_update(&mgr->matching_scene);
            matching_scene_draw(&mgr->matching_scene);

            switch (result)
            {
                case MATCHING_RESULT_SUCCESS:
                    // ゲームシーンへ遷移
                    return scene_change(mgr, SCENE_GAME);
                case MATCHING_RESULT_CANCEL:
                    // タイトルへ戻る
                    return scene_change(mgr, SCENE_TITLE);
                default:
                    return true;
            }
        }

        case SCENE_GAME:
        {
            GameSceneResult result =
                game_scene_update(&mgr->game_scene, player_input, player_swing);
            game_scene_draw(&mgr->game_scene);

            switch (result)
            {
                case GAME_RESULT_CONTINUE:
                    return true;
                case GAME_RESULT_FINISHED:
                    if (mgr->network_initialized)
                    {
                        network_fini(&mgr->network);
                        mgr->network_initialized = false;
                    }
                    break;
                case GAME_RESULT_RETURN_TITLE:
                    if (mgr->network_initialized)
                    {
                        network_fini(&mgr->network);
                        mgr->network_initialized = false;
                    }
                    return scene_change(mgr, SCENE_TITLE);
                case GAME_RESULT_NETWORK_ERROR:
                    // ネットワークエラー時はネットワークを終了してタイトルに戻る
                    LOG_WARN("ネットワークエラーが発生しました。タイトルに戻ります。");
                    if (mgr->network_initialized)
                    {
                        network_fini(&mgr->network);
                        mgr->network_initialized = false;
                    }
                    return scene_change(mgr, SCENE_TITLE);
                default:
                    LOG_ERROR("不正なゲームシーン結果が返されました");
                    return false;
            }
        }

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

        case SCENE_MATCHING:
            LOG_DEBUG("SCENE_MATCHINGを終了します。");
            matching_scene_fini(&mgr->matching_scene);
            break;

        case SCENE_GAME:
            LOG_DEBUG("SCENE_GAMEを終了します。");
            game_scene_fini(&mgr->game_scene);
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