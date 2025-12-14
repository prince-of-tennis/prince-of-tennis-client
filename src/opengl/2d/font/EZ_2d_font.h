#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include <cstdint>
#include <map>
#include <memory>

#include "glad/glad.h"
#include "glm/glm.hpp"

using namespace std;

// テキスト描画用の文字情報
struct Character
{
    GLuint texture_id;   // グリフのテクスチャID
    glm::ivec2 size;     // グリフのサイズ
    glm::ivec2 bearing;  // ベースラインからのオフセット
    GLuint advance;      // 次の文字への距離
};

// フォント構造体
struct _EZ_2D_Font
{
    FT_Face face;
    map<uint32_t, Character> characters;  // UTF-8対応: charからuint32_tに変更
    int font_size;

    ~_EZ_2D_Font();
};

typedef std::shared_ptr<_EZ_2D_Font> EZ_2D_Font;

/// @brief フォントの読み込み
/// @param font_path フォントファイルのパス
/// @param font_size フォントサイズ（デフォルト48）
/// @return 読み込まれたフォント
EZ_2D_Font EZ_2D_CreateFont(const char *font_path, int font_size = 48);

/// @brief フォントの解放
/// @param font フォント構造体
void _EZ_2D_DestroyFont(_EZ_2D_Font *font);

/// @brief UTF-8文字列から次の1文字のコードポイントを取得
/// @param text UTF-8文字列のポインタ（参照渡しで進められる）
/// @return Unicodeコードポイント
uint32_t _EZ_2D_GetNextUTF8Char(const char **text);

/// @brief 指定されたコードポイントのグリフを読み込んでキャッシュ
/// @param font フォント構造体
/// @param codepoint Unicodeコードポイント
/// @return 成功したかどうか
bool _EZ_2D_LoadGlyph(_EZ_2D_Font *font, uint32_t codepoint);

/// @brief テキストの描画
/// @param font 使用するフォント
/// @param x X座標 (左上原点)
/// @param y Y座標 (左上原点)
/// @param text 描画するテキスト
/// @param size フォントサイズ
/// @param r 赤成分 (0.0~1.0)
/// @param g 緑成分 (0.0~1.0)
/// @param b 青成分 (0.0~1.0)
/// @param a アルファ成分 (0.0~1.0)
void EZ_2D_DrawText(EZ_2D_Font font, float x, float y, const char *text, float size, float r,
                    float g, float b, float a);
