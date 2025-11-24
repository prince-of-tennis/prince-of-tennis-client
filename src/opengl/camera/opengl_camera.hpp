#pragma once

#include <glm/glm.hpp>

struct OpenGLShader;
struct OpenGLLight;
struct OpenGLObject;

struct OpenGLCamera
{
    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 up;

    float fov;
    float aspect_ratio;
    float near_plane;
    float far_plane;
};

void opengl_camera_init(OpenGLCamera *camera, float aspect_ratio);
void opengl_camera_update(OpenGLCamera *camera);
void opengl_camera_set_position(OpenGLCamera *camera, glm::vec3 position);
void opengl_camera_set_target(OpenGLCamera *camera, glm::vec3 target);
void opengl_camera_set_fov(OpenGLCamera *camera, float fov);

glm::mat4 opengl_camera_get_view_matrix(OpenGLCamera *camera);
glm::mat4 opengl_camera_get_projection_matrix(OpenGLCamera *camera);

void opengl_camera_draw(OpenGLCamera *camera, OpenGLShader *shader, OpenGLLight *light,
                        OpenGLObject *object);
