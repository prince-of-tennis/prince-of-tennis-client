#include "game_scene.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glad/glad.h"
#include "util/log.hpp"

bool game_scene_init(GameScene *scene)
{
    LOG_DEBUG("GameScene初期化開始");

    // シェーダー初期化
    if (!opengl_shader_init(&scene->shader, "shader/simple.vert", "shader/simple.frag"))
    {
        LOG_ERROR("シェーダーの初期化に失敗しました");
        return false;
    }
    LOG_DEBUG("シェーダー初期化完了");

    // オブジェクト作成
    scene->obj.reset(opengl_object_create("obj/object.obj", "img/container.jpeg"));
    if (scene->obj == nullptr)
    {
        LOG_ERROR("オブジェクトの作成に失敗しました");
        return false;
    }

    // オブジェクトの位置を設定
    opengl_object_set_position(scene->obj.get(), glm::vec3(0.0f, 0.0f, -5.0f));

    // カメラ初期化
    opengl_camera_init(&scene->camera, 800.0f / 600.0f);
    opengl_camera_set_position(&scene->camera, glm::vec3(0.0f, 2.0f, 10.0f));
    opengl_camera_set_target(&scene->camera, glm::vec3(0.0f, 0.0f, 0.0f));

    // ライト初期化
    opengl_light_init(&scene->light);

    // 深度テスト有効化
    glEnable(GL_DEPTH_TEST);

    LOG_DEBUG("GameScene初期化完了");
    return true;
}

bool game_scene_update(GameScene *scene)
{
    // オブジェクトを回転させる
    static float rotation_angle = 0.0f;
    rotation_angle += 1.0f;  // 毎フレーム1度回転

    if (scene->obj)
    {
        opengl_object_set_rotation(scene->obj.get(), glm::vec3(0.0f, rotation_angle, 0.0f));
    }

    // カメラ更新
    opengl_camera_update(&scene->camera);

    // ライト更新
    opengl_light_update(&scene->light);

    return true;
}

void game_scene_draw(GameScene *scene)
{
    // 画面クリア
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (scene->obj)
    {
        opengl_camera_draw(&scene->camera, &scene->shader, &scene->light, scene->obj.get());
    }
    else
    {
        LOG_ERROR("描画対象のオブジェクトがnullです");
    }
}