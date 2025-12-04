#include "opengl_object.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include "util/log.hpp"

OpenGLObject *opengl_object_create(const char *model_file, const char *texture_file)
{
    LOG_DEBUG("OpenGLObject作成開始: model=" << model_file << ", texture=" << texture_file);
    OpenGLObject *obj = new OpenGLObject();

    if (!opengl_model_init(&obj->model, model_file))
    {
        LOG_ERROR("モデルの初期化に失敗しました: " << model_file);
        delete obj;
        return nullptr;
    }
    LOG_SUCCESS("モデルの初期化に成功しました");

    if (!opengl_texture_init(&obj->texture, texture_file))
    {
        LOG_ERROR("テクスチャの初期化に失敗しました: " << texture_file);
        opengl_model_destroy(&obj->model);
        delete obj;
        return nullptr;
    }
    LOG_SUCCESS("テクスチャの初期化に成功しました");

    obj->transform.position = glm::vec3(0.0f, 0.0f, 0.0f);
    obj->transform.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    obj->transform.scale = glm::vec3(1.0f, 1.0f, 1.0f);
    obj->is_active = true;

    LOG_SUCCESS("OpenGLObject作成完了");
    return obj;
}

void opengl_object_destroy(OpenGLObject *obj)
{
    opengl_model_destroy(&obj->model);
    opengl_texture_destroy(&obj->texture);
    obj->is_active = false;
}

void opengl_object_set_position(OpenGLObject *obj, glm::vec3 pos)
{
    obj->transform.position = pos;
}

void opengl_object_set_rotation(OpenGLObject *obj, glm::vec3 rot)
{
    obj->transform.rotation = rot;
}

void opengl_object_set_scale(OpenGLObject *obj, glm::vec3 scale)
{
    obj->transform.scale = scale;
}

glm::mat4 opengl_object_get_model_matrix(OpenGLObject *obj)
{
    glm::mat4 model = glm::mat4(1.0f);

    model = glm::translate(model, obj->transform.position);

    model =
        glm::rotate(model, glm::radians(obj->transform.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    model =
        glm::rotate(model, glm::radians(obj->transform.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model =
        glm::rotate(model, glm::radians(obj->transform.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

    model = glm::scale(model, obj->transform.scale);

    return model;
}

void opengl_object_draw(OpenGLObject *obj)
{
    if (!obj->is_active)
    {
        LOG_DEBUG("オブジェクトが非アクティブのため描画をスキップ");
        return;
    }

    opengl_texture_bind(&obj->texture, 0);
    opengl_model_draw(&obj->model);
}
