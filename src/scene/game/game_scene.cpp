#include "game_scene.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glad/glad.h"
#include "network/network.hpp"
#include "util/log.hpp"

bool game_scene_init(GameScene *scene)
{
    LOG_DEBUG("GameScene初期化開始");

    // シェーダー初期化
    scene->shader = EZ_CreateShader("shader/simple.vert", "shader/simple.frag");
    if (scene->shader == nullptr)
    {
        LOG_ERROR("シェーダーの作成に失敗しました");
        return false;
    }
    LOG_SUCCESS("シェーダー初期化完了");

    EZ_Model model = EZ_CreateModel("obj/tennis_court.obj");
    EZ_Texture texture = EZ_CreateTexture("img/container.jpeg");

    // オブジェクト作成
    scene->object = EZ_CreateObjectFromModelTexture(model, texture);
    if (scene->object == nullptr)
    {
        LOG_ERROR("オブジェクトの作成に失敗しました");
        return false;
    }
    // オブジェクトの位置を設定
    EZ_ObjectSetPosition(scene->object, 0.0f, -1.0f, 0.0f);

    // カメラ初期化
    scene->camera = EZ_CreateCamera(
        static_cast<float>(scene->context->window_width / scene->context->window_height));

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

bool game_scene_update(GameScene *scene)
{
    if (!network_listen_to_server(scene->network.get()))
    {
        return false;
    }

    return true;
}

void game_scene_draw(GameScene *scene)
{
    // 画面クリア
    glClearColor(scene->context->background_r, scene->context->background_g,
                 scene->context->background_b, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (scene->object)
    {
        EZ_DrawObject(scene->object, scene->shader, scene->camera, scene->light);
    }
    else
    {
        LOG_ERROR("描画対象のオブジェクトがnullです");
    }
}