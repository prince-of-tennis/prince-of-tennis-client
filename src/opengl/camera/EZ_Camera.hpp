#pragma once

#include <glm/glm.hpp>
#include <memory>

struct _EZ_Camera
{
    glm::vec3 position;  // カメラの位置
    glm::vec3 target;    // カメラの注視点
    glm::vec3 up;        // カメラの上方向ベクトル

    float fov;           // 視野角 (Field of View)
    float aspect_ratio;  // カメラのアスペクト比
    float near_plane;    // ニアクリップ平面(描画する範囲の最小距離)
    float far_plane;     // ファークリップ平面(描画する範囲の最大距離)
};

typedef std::shared_ptr<_EZ_Camera> EZ_Camera;

/// @brief カメラの作成
/// @param aspect_ratio アスペクト比(幅 / 高さ)
/// @return 作成されたカメラ
EZ_Camera EZ_CreateCamera(float aspect_ratio);

/// @brief カメラの座標設定
/// @param camera カメラ構造体
/// @param x X座標
/// @param y Y座標
/// @param z Z座標
void EZ_CameraSetPosition(EZ_Camera camera, float x, float y, float z);

/// @brief カメラの注視点設定
/// @param camera カメラ構造体
/// @param x 注視点X座標
/// @param y 注視点Y座標
/// @param z 注視点Z座標
void EZ_CameraSetTargetPosition(EZ_Camera camera, float x, float y, float z);

/// @brief カメラの視野角設定
/// @param camera カメラ構造体
/// @param fov 視野角 (Field of View)
void EZ_CameraSetFov(EZ_Camera camera, float fov);

/// @brief カメラの描画範囲設定
/// @param camera カメラ構造体
/// @param near_plane ニアクリップ平面
/// @param far_plane ファークリップ平面
void EZ_CameraSetClipPlanes(EZ_Camera camera, float near_plane, float far_plane);

glm::mat4 _EZ_CameraGetViewMatrix(_EZ_Camera *camera);
glm::mat4 _EZ_CameraGetProjectionMatrix(_EZ_Camera *camera);
