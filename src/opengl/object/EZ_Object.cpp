#include "EZ_Object.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include "glm/gtc/type_ptr.hpp"
#include "util/log.hpp"

EZ_Object EZ_CreateObject(const char *model_file, const char *texture_file)
{
    EZ_Model model = EZ_CreateModel(model_file);
    if (!model)
    {
        LOG_ERROR("モデルの初期化に失敗しました: " << model_file);
        return nullptr;
    }

    EZ_Texture texture = EZ_CreateTexture(texture_file);
    if (!texture)
    {
        LOG_ERROR("テクスチャの初期化に失敗しました: " << texture_file);
        return nullptr;
    }

    EZ_Object object = EZ_CreateObjectFromModelTexture(model, texture);
    if (!object)
    {
        LOG_ERROR("EZ_Objectの作成に失敗しました");
        return nullptr;
    }

    LOG_SUCCESS("EZ_Object作成完了");
    return object;
}

EZ_Object EZ_CreateObjectFromModel(EZ_Model model, const char *texture_file)
{
    if (!model)
    {
        LOG_ERROR("モデルがNULLです");
        return nullptr;
    }

    EZ_Texture texture = EZ_CreateTexture(texture_file);
    if (!texture)
    {
        LOG_ERROR("テクスチャの初期化に失敗しました: " << texture_file);
        return nullptr;
    }

    EZ_Object object = EZ_CreateObjectFromModelTexture(model, texture);
    if (!object)
    {
        LOG_ERROR("EZ_Objectの作成に失敗しました");
        return nullptr;
    }

    LOG_SUCCESS("EZ_Object作成完了");
    return object;
}

EZ_Object EZ_CreateObjectFromModelTexture(EZ_Model model, EZ_Texture texture)
{
    EZ_Object obj = make_shared<_EZ_Object>();
    if (!model)
    {
        LOG_ERROR("モデルがNULLです");
        return nullptr;
    }
    obj->model = model;

    if (!texture)
    {
        LOG_ERROR("テクスチャがNULLです");
        return nullptr;
    }
    obj->texture = texture;

    obj->transform.position = glm::vec3(0.0f, 0.0f, 0.0f);
    obj->transform.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    obj->transform.scale = glm::vec3(1.0f, 1.0f, 1.0f);
    obj->is_active = true;

    LOG_SUCCESS("EZ_Object作成完了");
    return obj;
}

void _EZ_DestroyObject(_EZ_Object *obj)
{
    obj->is_active = false;
}

void EZ_ObjectSetPosition(EZ_Object obj, float x, float y, float z)
{
    obj->transform.position = glm::vec3(x, y, z);
}

void EZ_ObjectSetRotation(EZ_Object obj, float x, float y, float z)
{
    obj->transform.rotation = glm::vec3(x, y, z);
}

void EZ_ObjectSetScale(EZ_Object obj, float x, float y, float z)
{
    obj->transform.scale = glm::vec3(x, y, z);
}

glm::mat4 _EZ_ObjectGetModelMatrix(_EZ_Object *obj)
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

void EZ_DrawObject(EZ_Object object, EZ_Shader shader, EZ_Camera camera, EZ_Light light)
{
    // シェーダー使用
    _EZ_UseShader(shader.get());

    // カメラ行列設定
    glm::mat4 projection = _EZ_CameraGetProjectionMatrix(camera.get());
    glm::mat4 view = _EZ_CameraGetViewMatrix(camera.get());

    glUniformMatrix4fv(shader->proj_loc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(shader->view_loc, 1, GL_FALSE, glm::value_ptr(view));

    // ライト設定
    glUniform3fv(shader->light_loc, 1, glm::value_ptr(light->position));
    glUniform3fv(shader->light_color_loc, 1, glm::value_ptr(light->color));
    glUniform3fv(shader->view_pos_loc, 1, glm::value_ptr(camera->position));

    // モデル行列設定と描画
    glm::mat4 model_matrix = _EZ_ObjectGetModelMatrix(object.get());
    glUniformMatrix4fv(shader->model_loc, 1, GL_FALSE, glm::value_ptr(model_matrix));

    if (!object->is_active)
    {
        LOG_DEBUG("オブジェクトが非アクティブのため描画をスキップ");
        return;
    }

    _EZ_DrawModel(object->model.get(), object->texture.get());
}

_EZ_Object::~_EZ_Object()
{
    _EZ_DestroyObject(this);
}
