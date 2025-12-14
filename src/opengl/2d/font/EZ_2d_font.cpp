#include "EZ_2d_font.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <cstdint>
#include <string>
#include <vector>

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "util/log.hpp"

using namespace std;

// FreeTypeライブラリ
static FT_Library g_ft_library = nullptr;
static bool g_ft_initialized = false;

// 不正なUTF-8シーケンスをスキップするヘルパー関数
static void skip_invalid_utf8_sequence(const char **text)
{
    (*text)++;
    while (**text != 0 && (((unsigned char)**text & 0xC0) == 0x80))
    {
        // 継続バイトをスキップ
        (*text)++;
    }
}

// UTF-8文字列から次の1文字のコードポイントを取得
uint32_t _EZ_2D_GetNextUTF8Char(const char **text)
{
    const unsigned char *bytes = (const unsigned char *)*text;
    uint32_t codepoint = 0;
    int bytes_to_read = 0;

    if (bytes[0] == 0)
    {
        return 0;  // 文字列の終端
    }
    else if ((bytes[0] & 0x80) == 0x00)
    {
        // 1バイト文字 (ASCII)
        codepoint = bytes[0];
        bytes_to_read = 1;
    }
    else if ((bytes[0] & 0xE0) == 0xC0)
    {
        // 2バイト文字
        codepoint = bytes[0] & 0x1F;
        bytes_to_read = 2;
    }
    else if ((bytes[0] & 0xF0) == 0xE0)
    {
        // 3バイト文字
        codepoint = bytes[0] & 0x0F;
        bytes_to_read = 3;
    }
    else if ((bytes[0] & 0xF8) == 0xF0)
    {
        // 4バイト文字
        codepoint = bytes[0] & 0x07;
        bytes_to_read = 4;
    }
    else
    {
        // 不正なUTF-8シーケンス
        // 継続バイト(0x80-0xBF)または無効な開始バイト(0xF8以上)の場合、
        // 次の有効なUTF-8シーケンス開始位置までスキップ
        skip_invalid_utf8_sequence(text);
        return 0xFFFD;  // 置換文字
    }

    // 残りのバイトを読み込む
    for (int i = 1; i < bytes_to_read; i++)
    {
        if ((bytes[i] & 0xC0) != 0x80)
        {
            // 不正なUTF-8シーケンス（継続バイトが期待される位置に無効なバイト）
            // 次の有効なUTF-8シーケンス開始位置までスキップ
            skip_invalid_utf8_sequence(text);
            return 0xFFFD;
        }
        codepoint = (codepoint << 6) | (bytes[i] & 0x3F);
    }

    *text += bytes_to_read;
    return codepoint;
}

extern GLuint g_shader_program;
extern GLint g_uniform_projection;
extern GLint g_uniform_model;
extern GLint g_uniform_color;
extern GLint g_uniform_use_texture;
extern GLint g_uniform_is_text;
extern GLuint g_text_vao;
extern GLuint g_text_vbo;
extern glm::mat4 g_projection_matrix;

// FreeTypeライブラリの初期化
bool _EZ_2D_InitFreeType()
{
    if (!g_ft_initialized)
    {
        if (FT_Init_FreeType(&g_ft_library))
        {
            LOG_ERROR("FreeTypeライブラリの初期化に失敗");
            return false;
        }
        g_ft_initialized = true;
        LOG_SUCCESS("FreeTypeライブラリ初期化完了");
    }
    return true;
}

void _EZ_2D_DestroyFreeType()
{
    if (g_ft_initialized && g_ft_library)
    {
        FT_Done_FreeType(g_ft_library);
        g_ft_library = nullptr;
        g_ft_initialized = false;
    }
}

EZ_2D_Font EZ_2D_CreateFont(const char *font_path, int font_size)
{
    if (!g_ft_initialized)
    {
        LOG_ERROR("2D描画システムが初期化されていません");
        return nullptr;
    }

    auto font = make_shared<_EZ_2D_Font>();
    font->font_size = font_size;

    if (FT_New_Face(g_ft_library, font_path, 0, &font->face))
    {
        LOG_ERROR("フォントの読み込みに失敗: " << font_path);
        return nullptr;
    }

    FT_Set_Pixel_Sizes(font->face, 0, font_size);

    // Unicodeチャーマップを設定
    if (FT_Select_Charmap(font->face, FT_ENCODING_UNICODE))
    {
        LOG_ERROR("Unicodeチャーマップの設定に失敗: " << font_path);
        FT_Done_Face(font->face);
        return nullptr;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    LOG_SUCCESS("フォント読み込み完了: " << font_path << " (size=" << font_size << ")");
    return font;
}

// 指定されたコードポイントのグリフをロードしてキャッシュ
bool _EZ_2D_LoadGlyph(_EZ_2D_Font *font, uint32_t codepoint)
{
    if (!font || !font->face)
    {
        return false;
    }

    // 既にキャッシュされているかチェック
    if (font->characters.find(codepoint) != font->characters.end())
    {
        return true;
    }

    // FreeTypeでグリフをロード
    FT_UInt glyph_index = FT_Get_Char_Index(font->face, codepoint);
    if (glyph_index == 0)
    {
        // グリフが存在しない場合
        return false;
    }

    if (FT_Load_Glyph(font->face, glyph_index, FT_LOAD_RENDER))
    {
        LOG_WARN("グリフの読み込みに失敗: U+" << std::hex << codepoint);
        return false;
    }

    int width = font->face->glyph->bitmap.width;
    int height = font->face->glyph->bitmap.rows;
    unsigned char *buffer = font->face->glyph->bitmap.buffer;

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, buffer);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // キャラクター情報をキャッシュ
    Character character = {
        texture, glm::ivec2(width, height),
        glm::ivec2(font->face->glyph->bitmap_left, font->face->glyph->bitmap_top),
        static_cast<GLuint>(font->face->glyph->advance.x)};

    font->characters.insert(pair<uint32_t, Character>(codepoint, character));
    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
}

void _EZ_2D_DestroyFont(_EZ_2D_Font *font)
{
    if (!font)
    {
        LOG_ERROR("fontがNULLです。");
        return;
    }

    // テクスチャの解放
    for (auto &pair : font->characters)
    {
        glDeleteTextures(1, &pair.second.texture_id);
    }
    font->characters.clear();

    // FreeType Faceの解放
    if (font->face)
    {
        FT_Done_Face(font->face);
        font->face = nullptr;
    }

    LOG_SUCCESS("フォント解放完了");
}

_EZ_2D_Font::~_EZ_2D_Font()
{
    _EZ_2D_DestroyFont(this);
}

void EZ_2D_DrawText(EZ_2D_Font font, float x, float y, const char *text, float size, float r,
                    float g, float b, float a)
{
    if (!font || !text)
    {
        return;
    }

    glUseProgram(g_shader_program);

    glUniformMatrix4fv(g_uniform_projection, 1, GL_FALSE, glm::value_ptr(g_projection_matrix));
    glUniform4f(g_uniform_color, r, g, b, a);
    glUniform1i(g_uniform_use_texture, 1);
    glUniform1i(g_uniform_is_text, 1);  // テキスト描画

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(g_text_vao);

    float scale = size / (float)font->font_size;  // フォントサイズからスケール
    const char *p = text;

    // UTF-8文字列をパースしながらループ
    while (*p)
    {
        uint32_t codepoint = _EZ_2D_GetNextUTF8Char(&p);

        if (codepoint == 0)
        {
            break;  // 文字列の終端
        }

        // グリフがキャッシュされていなければロード
        if (font->characters.find(codepoint) == font->characters.end())
        {
            if (!_EZ_2D_LoadGlyph(font.get(), codepoint))
            {
                // ロードに失敗した場合はスキップ
                continue;
            }
        }

        Character ch = font->characters[codepoint];

        // ベースラインに合わせた位置計算
        float xpos = x + ch.bearing.x * scale;
        float ypos = y - ch.bearing.y * scale;  // ベースライン基準の座標計算
        float w = ch.size.x * scale;
        float h = ch.size.y * scale;

        float vertices[6][4] = {
            {xpos, ypos + h, 0.0f, 1.0f},  // 左上
            {xpos, ypos, 0.0f, 0.0f},      // 左下
            {xpos + w, ypos, 1.0f, 0.0f},  // 右下

            {xpos, ypos + h, 0.0f, 1.0f},     // 左上
            {xpos + w, ypos, 1.0f, 0.0f},     // 右下
            {xpos + w, ypos + h, 1.0f, 1.0f}  // 右上
        };

        glBindTexture(GL_TEXTURE_2D, ch.texture_id);

        auto model = glm::mat4(1.0f);
        glUniformMatrix4fv(g_uniform_model, 1, GL_FALSE, glm::value_ptr(model));

        glBindBuffer(GL_ARRAY_BUFFER, g_text_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        x += (ch.advance >> 6) * scale;  // 次の文字位置へ移動
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
