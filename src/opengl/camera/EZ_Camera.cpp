#include "EZ_Camera.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glad/glad.h"
#include "opengl/light/EZ_Light.hpp"
#include "opengl/object/EZ_Object.hpp"
#include "opengl/shader/EZ_Shader.hpp"

EZ_Camera EZ_CreateCamera(float aspect_ratio)
{
    auto camera = std::make_shared<_EZ_Camera>();

    camera->position = glm::vec3(0.0f, 0.0f, 10.0f);
    camera->target = glm::vec3(0.0f, 0.0f, 0.0f);
    camera->up = glm::vec3(0.0f, 1.0f, 0.0f);

    camera->fov = 45.0f;
    camera->aspect_ratio = aspect_ratio;
    camera->near_plane = 0.1f;
    camera->far_plane = 100.0f;

    return camera;
}

void EZ_CameraSetPosition(EZ_Camera camera, float x, float y, float z)
{
    camera->position = glm::vec3(x, y, z);
}

void EZ_CameraSetTargetPosition(EZ_Camera camera, float x, float y, float z)
{
    camera->target = glm::vec3(x, y, z);
}

void EZ_CameraSetFov(EZ_Camera camera, float fov)
{
    camera->fov = fov;
}

glm::mat4 _EZ_CameraGetViewMatrix(_EZ_Camera *camera)
{
    return glm::lookAt(camera->position, camera->target, camera->up);
}

glm::mat4 _EZ_CameraGetProjectionMatrix(_EZ_Camera *camera)
{
    return glm::perspective(glm::radians(camera->fov), camera->aspect_ratio, camera->near_plane,
                            camera->far_plane);
}
