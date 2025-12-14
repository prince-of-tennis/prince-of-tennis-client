#pragma once

#include <memory>

#include "glad/glad.h"

using namespace std;

// 画像構造体
struct _EZ_2D_Image
{
    GLuint texture_id;  // テクスチャID
    int width;          // 画像の幅
    int height;         // 画像の高さ

    ~_EZ_2D_Image();
};

typedef std::shared_ptr<_EZ_2D_Image> EZ_2D_Image;

/// @brief 画像の読み込み
/// @param image_path 画像ファイルのパス
/// @return 読み込まれた画像
EZ_2D_Image EZ_2D_CreateImage(const char *image_path);

/// @brief 画像の解放
/// @param image 画像構造体
void _EZ_2D_DestroyImage(_EZ_2D_Image *image);

/// @brief 画像の描画
/// @param image 使用する画像
/// @param x X座標 (左上原点)
/// @param y Y座標 (左上原点)
/// @param width 幅（0の場合は元のサイズ）
/// @param height 高さ（0の場合は元のサイズ）
/// @param r 赤成分 (0.0~1.0) 色調整用
/// @param g 緑成分 (0.0~1.0) 色調整用
/// @param b 青成分 (0.0~1.0) 色調整用
/// @param a アルファ成分 (0.0~1.0)
void EZ_2D_DrawImage(EZ_2D_Image image, float x, float y, float width, float height, float r,
                     float g, float b, float a);
