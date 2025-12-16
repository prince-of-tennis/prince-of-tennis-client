#include "game_scene.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glad/glad.h"
#include "network/network.hpp"
#include "opengl/2d/EZ_2d.h"
#include "opengl/EasyGL.hpp"
#include "util/log.hpp"

bool game_scene_init(GameScene *scene)
{
    LOG_DEBUG("GameScene初期化開始");

    // シェーダー初期化
    scene->shader = EZ_CreateShader();
    if (scene->shader == nullptr)
    {
        LOG_ERROR("シェーダーの作成に失敗しました");
        return false;
    }
    LOG_SUCCESS("シェーダー初期化完了");

    // ボールオブジェクトの初期化
    scene->ball_object = EZ_CreateObject("obj/ball.obj", "img/container.jpeg");
    if (scene->ball_object == nullptr)
    {
        LOG_ERROR("ボールオブジェクトの作成に失敗しました");
        return false;
    }

    // ボールの初期座標を設定（テスト用）
    scene->ball_data.point.x = 0.0f;
    scene->ball_data.point.y = 2.0f;
    scene->ball_data.point.z = 0.0f;

    EZ_ObjectSetPosition(scene->ball_object, scene->ball_data.point.x, scene->ball_data.point.y,
                         scene->ball_data.point.z);
    EZ_ObjectSetScale(scene->ball_object, 0.2, 0.2, 0.2);

    LOG_SUCCESS("ボールオブジェクト初期化完了");

    // プレイヤーオブジェクトの初期化
    for (int i = 0; i < 2; i++)
    {
        scene->player_objects[i] = EZ_CreateObject("obj/player.obj", "img/container.jpeg");
        if (scene->player_objects[i] == nullptr)
        {
            LOG_ERROR("プレイヤーオブジェクト" << i << "の作成に失敗しました");
            return false;
        }
    }

    // プレイヤーの初期座標を設定（テスト用）
    // プレイヤー1（左側）
    scene->player_data[0].point.x = 0.0f;
    scene->player_data[0].point.y = 0.0f;
    scene->player_data[0].point.z = 0.0f;
    EZ_ObjectSetPosition(scene->player_objects[0], scene->player_data[0].point.x,
                         scene->player_data[0].point.y, scene->player_data[0].point.z);
    EZ_ObjectSetScale(scene->player_objects[0], 1.0, 1.0, 1.0);

    // プレイヤー2（右側）
    scene->player_data[1].point.x = 10.0f;
    scene->player_data[1].point.y = 0.0f;
    scene->player_data[1].point.z = 0.0f;
    EZ_ObjectSetPosition(scene->player_objects[1], scene->player_data[1].point.x,
                         scene->player_data[1].point.y, scene->player_data[1].point.z);
    EZ_ObjectSetScale(scene->player_objects[1], 1.0, 1.0, 1.0);

    LOG_SUCCESS("プレイヤーオブジェクト初期化完了");

    // カメラ初期化
    scene->camera = EZ_CreateCamera(static_cast<float>(scene->context->window_width) /
                                    static_cast<float>(scene->context->window_height));

    // ライト初期化
    scene->light = EZ_CreateLight();

    // Network
    scene->network.reset(new Network);
    if (!network_init(scene->network.get(), scene->context))
    {
        return false;
    }

    LOG_SUCCESS("GameScene初期化完了");
    return true;
}

bool game_scene_update(GameScene *scene, PlayerInput *player_input)
{
    if (!network_listen_to_server(scene->network.get()))
    {
        return false;
    }

    // ネットワークからボールのデータを取得して反映
    scene->ball_data = scene->network->network_data_set.ball;

    // ボールオブジェクトの座標を更新
    EZ_ObjectSetPosition(scene->ball_object, scene->ball_data.point.x, scene->ball_data.point.y,
                         scene->ball_data.point.z);

    // プレイヤーの座標を更新（将来的にネットワークから取得）
    for (int i = 0; i < PLAYER_MAX; i++)
    {
        EZ_ObjectSetPosition(scene->player_objects[i], scene->player_data[i].point.x,
                             scene->player_data[i].point.y, scene->player_data[i].point.z);
    }

    return true;
}

void game_scene_draw(GameScene *scene)
{
    // 画面クリア
    EZ_BackgroundClear(scene->context->background_r, scene->context->background_g,
                       scene->context->background_b);

    // ボールの描画
    if (scene->ball_object)
    {
        EZ_DrawObject(scene->ball_object, scene->shader, scene->camera, scene->light);
    }

    // プレイヤーの描画
    for (int i = 0; i < PLAYER_MAX; i++)
    {
        if (scene->player_objects[i])
        {
            EZ_DrawObject(scene->player_objects[i], scene->shader, scene->camera, scene->light);
        }
    }
}