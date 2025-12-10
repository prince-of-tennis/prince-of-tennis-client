#pragma once
#include <glm/glm.hpp>
#include <memory>

struct _EZ_Light
{
    glm::vec3 position;
    glm::vec3 color;
    float ambient_strength;
    float specular_strength;
};

/// @brief ライト
typedef std::shared_ptr<_EZ_Light> EZ_Light;

/// @brief ライトの作成
/// @return 作成されたライト
EZ_Light EZ_CreateLight();

/// @brief ライトの位置設定
/// @param light ライト構造体
/// @param x X座標
/// @param y Y座標
/// @param z Z座標
void EZ_LightSetPosition(EZ_Light light, float x, float y, float z);

/// @brief ライトの色設定
/// @param light ライト構造体
/// @param r 赤成分 (0-255)
/// @param g 緑成分 (0-255)
/// @param b 青成分 (0-255)
void EZ_LightSetColor(EZ_Light light, int r, int g, int b);
/// @brief ライトの環境光強度設定
/// @param light ライト構造体
/// @param strength 環境光強度 (0.0 - 1.0)
void EZ_LightSetAmbientStrength(EZ_Light light, float strength);

/// @brief ライトの鏡面反射強度設定
/// @param light ライト構造体
/// @param strength 鏡面反射強度 (0.0 - 1.0)
void EZ_LightSetSpecularStrength(EZ_Light light, float strength);
