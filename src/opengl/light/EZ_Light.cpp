#include "EZ_Light.hpp"

EZ_Light EZ_CreateLight()
{
    EZ_Light light = std::make_shared<_EZ_Light>();
    light->position = glm::vec3(5.0f, 5.0f, 5.0f);
    light->color = glm::vec3(1.0f, 1.0f, 1.0f);
    light->ambient_strength = 0.3f;
    light->specular_strength = 0.5f;

    return light;
}

void EZ_LightSetPosition(EZ_Light light, float x, float y, float z)
{
    light->position = glm::vec3(x, y, z);
}

void EZ_LightSetColor(EZ_Light light, int r, int g, int b)
{
    light->color = glm::vec3(r / 225.0f, g / 225.0f, b / 225.0f);
}

void EZ_LightSetAmbientStrength(EZ_Light light, float strength)
{
    light->ambient_strength = strength;
}

void EZ_LightSetSpecularStrength(EZ_Light light, float strength)
{
    light->specular_strength = strength;
}
