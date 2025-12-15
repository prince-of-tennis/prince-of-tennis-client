#include "game_scene.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glad/glad.h"
#include "network/network.hpp"
#include "opengl/2d/EZ_2d.h"
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

bool game_scene_update(GameScene *scene, PlayerInput player_input)
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

    return true;
}

void game_scene_draw(GameScene *scene)
{
    // 画面クリア
    glClearColor(scene->context->background_r, scene->context->background_g,
                 scene->context->background_b, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // ボールの描画
    if (scene->ball_object)
    {
        EZ_DrawObject(scene->ball_object, scene->shader, scene->camera, scene->light);
    }
}