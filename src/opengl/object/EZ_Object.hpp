#pragma once

#include <glm/glm.hpp>
#include <memory>

#include "opengl/camera/EZ_Camera.hpp"
#include "opengl/light/EZ_Light.hpp"
#include "opengl/model/EZ_Model.hpp"
#include "opengl/shader/EZ_Shader.hpp"
#include "opengl/texture/EZ_Texture.hpp"

using namespace std;

struct Transform
{
    glm::vec3 position;
    glm::vec3 rotation;  // Euler angles (度数法)
    glm::vec3 scale;
};

struct _EZ_Object
{
    EZ_Model model;       // モデル構造体
    EZ_Texture texture;   // テクスチャ構造体
    Transform transform;  // 変換情報
    bool is_active;       // オブジェクトがアクティブかどうか

    ~_EZ_Object();
};

typedef std::shared_ptr<_EZ_Object> EZ_Object;

/// @brief オブジェクトの作成
/// @param model_file モデルファイルパス
/// @param texture_file テクスチャファイルパス
/// @return 作成されたオブジェクト
EZ_Object EZ_CreateObject(const char *model_file, const char *texture_file);

/// @brief モデルからオブジェクトを作成
/// @param model モデル構造体
/// @param texture_file テクスチャファイルパス
/// @return 作成されたオブジェクト
EZ_Object EZ_CreateObjectFromModel(EZ_Model model, const char *texture_file);

/// @brief モデルとテクスチャからオブジェクトを作成
/// @param model モデル構造体
/// @param texture テクスチャ構造体
/// @return 作成されたオブジェクト
EZ_Object EZ_CreateObjectFromModelTexture(EZ_Model model, EZ_Texture texture);

/// @brief 位置を設定
/// @param obj オブジェクト構造体
/// @param x X座標
/// @param y Y座標
/// @param z Z座標
void EZ_ObjectSetPosition(EZ_Object obj, float x, float y, float z);

/// @brief 回転を設定
/// @param obj オブジェクト構造体
/// @param x X軸回転 (度数法)
/// @param y Y軸回転 (度数法)
/// @param z Z軸回転 (度数法)
void EZ_ObjectSetRotation(EZ_Object obj, float x, float y, float z);

/// @brief スケールを設定
/// @param obj オブジェクト構造体
/// @param x X軸スケール
/// @param y Y軸スケール
/// @param z Z軸スケール
void EZ_ObjectSetScale(EZ_Object obj, float x, float y, float z);

/// @brief オブジェクトの描画
/// @param obj オブジェクト構造体
/// @param shader シェーダー構造体
/// @param camera カメラ構造体
/// @param light ライト構造体
void EZ_DrawObject(EZ_Object obj, EZ_Shader shader, EZ_Camera camera, EZ_Light light);

/// @brief オブジェクトの解放
void _EZ_DestroyObject(_EZ_Object *obj);

/// @brief モデル行列を取得
glm::mat4 _EZ_ObjectGetModelMatrix(_EZ_Object *obj);
