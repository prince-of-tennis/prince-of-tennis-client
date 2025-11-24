#include "opengl_light.hpp"

void opengl_light_init(OpenGLLight *light)
{
    light->position = glm::vec3(5.0f, 5.0f, 5.0f);
    light->color = glm::vec3(1.0f, 1.0f, 1.0f);
    light->ambient_strength = 0.3f;
    light->specular_strength = 0.5f;
}

void opengl_light_update(OpenGLLight *light)
{
    // ライトの更新処理が必要な場合はここに追加
    // 例: アニメーション、回転、明滅など
}

void opengl_light_set_position(OpenGLLight *light, glm::vec3 position)
{
    light->position = position;
}

void opengl_light_set_color(OpenGLLight *light, glm::vec3 color)
{
    light->color = color;
}

void opengl_light_set_ambient_strength(OpenGLLight *light, float strength)
{
    light->ambient_strength = strength;
}

void opengl_light_set_specular_strength(OpenGLLight *light, float strength)
{
    light->specular_strength = strength;
}
