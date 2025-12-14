#pragma once

#include <memory>

// サブモジュールのインクルード
#include "font/EZ_2d_font.h"
#include "image/EZ_2d_image.h"

/// @brief 2D描画システムの初期化
/// @param screen_width スクリーン幅
/// @param screen_height スクリーン高さ
/// @return 成功したか
bool EZ_2D_Init(int screen_width, int screen_height);

/// @brief 2D描画システムの解放
void _EZ_2D_Destroy();

/// @brief 矩形の描画
/// @param x X座標 (左上原点)
/// @param y Y座標 (左上原点)
/// @param width 幅
/// @param height 高さ
/// @param r 赤成分 (0.0~1.0)
/// @param g 緑成分 (0.0~1.0)
/// @param b 青成分 (0.0~1.0)
/// @param a アルファ成分 (0.0~1.0)
void EZ_2D_DrawRect(float x, float y, float width, float height, float r, float g, float b,
                    float a);

/// @brief 円の描画
/// @param x X座標 (中心)
/// @param y Y座標 (中心)
/// @param radius 半径
/// @param r 赤成分 (0.0~1.0)
/// @param g 緑成分 (0.0~1.0)
/// @param b 青成分 (0.0~1.0)
/// @param a アルファ成分 (0.0~1.0)
void EZ_2D_DrawCircle(float x, float y, float radius, float r, float g, float b, float a);
