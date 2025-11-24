#pragma once

#include <glm/glm.hpp>

struct OpenGLLight
{
    glm::vec3 position;
    glm::vec3 color;
    float ambient_strength;
    float specular_strength;
};

void opengl_light_init(OpenGLLight *light);
void opengl_light_update(OpenGLLight *light);
void opengl_light_set_position(OpenGLLight *light, glm::vec3 position);
void opengl_light_set_color(OpenGLLight *light, glm::vec3 color);
void opengl_light_set_ambient_strength(OpenGLLight *light, float strength);
void opengl_light_set_specular_strength(OpenGLLight *light, float strength);
