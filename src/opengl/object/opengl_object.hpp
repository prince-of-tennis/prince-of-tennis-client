#pragma once

#include <glm/glm.hpp>

#include "opengl/model/opengl_model.hpp"
#include "opengl/texture/opengl_texture.hpp"

using namespace std;

struct Transform
{
    glm::vec3 position;
    glm::vec3 rotation;  // Euler angles (度数法)
    glm::vec3 scale;
};

struct OpenGLObject
{
    OpenGLModel model;
    OpenGLTexture texture;
    Transform transform;
    bool is_active;
};

/// @brief オブジェクトの作成（ファクトリ関数）
/// @param model_file モデルファイルパス
/// @param texture_file テクスチャファイルパス
/// @param success 成功したかを格納するポインタ（オプション）
/// @return 作成されたOpenGLObject
OpenGLObject *opengl_object_create(const char *model_file, const char *texture_file);

/// @brief オブジェクトの解放
void opengl_object_destroy(OpenGLObject *obj);

/// @brief 位置を設定
void opengl_object_set_position(OpenGLObject *obj, glm::vec3 pos);

/// @brief 回転を設定
void opengl_object_set_rotation(OpenGLObject *obj, glm::vec3 rot);

/// @brief スケールを設定
void opengl_object_set_scale(OpenGLObject *obj, glm::vec3 scale);

/// @brief モデル行列を取得
glm::mat4 opengl_object_get_model_matrix(OpenGLObject *obj);

/// @brief オブジェクトの描画
void opengl_object_draw(OpenGLObject *obj);
