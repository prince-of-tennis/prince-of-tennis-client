#include "opengl_camera.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glad/glad.h"
#include "opengl/light/opengl_light.hpp"
#include "opengl/object/opengl_object.hpp"
#include "opengl/shader/opengl_shader.hpp"

void opengl_camera_init(OpenGLCamera *camera, float aspect_ratio)
{
    camera->position = glm::vec3(0.0f, 0.0f, 10.0f);
    camera->target = glm::vec3(0.0f, 0.0f, 0.0f);
    camera->up = glm::vec3(0.0f, 1.0f, 0.0f);

    camera->fov = 45.0f;
    camera->aspect_ratio = aspect_ratio;
    camera->near_plane = 0.1f;
    camera->far_plane = 100.0f;
}

void opengl_camera_update(OpenGLCamera *camera)
{
    // MARK: カメラの更新処理
}

void opengl_camera_set_position(OpenGLCamera *camera, glm::vec3 position)
{
    camera->position = position;
}

void opengl_camera_set_target(OpenGLCamera *camera, glm::vec3 target)
{
    camera->target = target;
}

void opengl_camera_set_fov(OpenGLCamera *camera, float fov)
{
    camera->fov = fov;
}

glm::mat4 opengl_camera_get_view_matrix(OpenGLCamera *camera)
{
    return glm::lookAt(camera->position, camera->target, camera->up);
}

glm::mat4 opengl_camera_get_projection_matrix(OpenGLCamera *camera)
{
    return glm::perspective(glm::radians(camera->fov), camera->aspect_ratio, camera->near_plane,
                            camera->far_plane);
}

void opengl_camera_draw(OpenGLCamera *camera, OpenGLShader *shader, OpenGLLight *light,
                        OpenGLObject *object)
{
    // シェーダー使用
    opengl_shader_use(shader);

    // カメラ行列設定
    glm::mat4 projection = opengl_camera_get_projection_matrix(camera);
    glm::mat4 view = opengl_camera_get_view_matrix(camera);

    glUniformMatrix4fv(shader->proj_loc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(shader->view_loc, 1, GL_FALSE, glm::value_ptr(view));

    // ライト設定
    glUniform3fv(shader->light_loc, 1, glm::value_ptr(light->position));
    glUniform3fv(shader->light_color_loc, 1, glm::value_ptr(light->color));
    glUniform3fv(shader->view_pos_loc, 1, glm::value_ptr(camera->position));

    // モデル行列設定と描画
    glm::mat4 model_matrix = opengl_object_get_model_matrix(object);
    glUniformMatrix4fv(shader->model_loc, 1, GL_FALSE, glm::value_ptr(model_matrix));

    opengl_object_draw(object);
}
