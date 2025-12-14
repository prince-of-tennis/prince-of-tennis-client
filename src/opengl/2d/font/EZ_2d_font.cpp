#include "EZ_2d_font.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <string>

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "util/log.hpp"

using namespace std;

// FreeTypeライブラリ
static FT_Library g_ft_library = nullptr;
static bool g_ft_initialized = false;

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

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // ASCII文字のグリフを事前にロード
    for (unsigned char c = 0; c < 128; c++)
    {
        if (FT_Load_Char(font->face, c, FT_LOAD_RENDER))
        {
            LOG_WARN("グリフの読み込みに失敗: " << (char)c);
            continue;
        }

        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, font->face->glyph->bitmap.width,
                     font->face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE,
                     font->face->glyph->bitmap.buffer);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Character character = {
            texture, glm::ivec2(font->face->glyph->bitmap.width, font->face->glyph->bitmap.rows),
            glm::ivec2(font->face->glyph->bitmap_left, font->face->glyph->bitmap_top),
            static_cast<GLuint>(font->face->glyph->advance.x)};

        font->characters.insert(pair<char, Character>(c, character));
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    LOG_SUCCESS("フォント読み込み完了: " << font_path << " (size=" << font_size << ")");
    return font;
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
    // フォントが読み込まれていない場合は何もしない
    if (!font || font->characters.empty())
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

    string::const_iterator c;
    for (c = string(text).begin(); c != string(text).end(); c++)
    {
        Character ch = font->characters[*c];

        float xpos = x + ch.bearing.x * scale;
        float ypos = y - (ch.size.y - ch.bearing.y) * scale;

        float w = ch.size.x * scale;
        float h = ch.size.y * scale;

        // 文字ごとのクアッドを更新
        float vertices[6][4] = {{xpos, ypos + h, 0.0f, 0.0f},    {xpos, ypos, 0.0f, 1.0f},
                                {xpos + w, ypos, 1.0f, 1.0f},

                                {xpos, ypos + h, 0.0f, 0.0f},    {xpos + w, ypos, 1.0f, 1.0f},
                                {xpos + w, ypos + h, 1.0f, 0.0f}};

        glBindTexture(GL_TEXTURE_2D, ch.texture_id);

        auto model = glm::mat4(1.0f);
        glUniformMatrix4fv(g_uniform_model, 1, GL_FALSE, glm::value_ptr(model));

        glBindBuffer(GL_ARRAY_BUFFER, g_text_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        x += (ch.advance >> 6) * scale;  // ビット単位で6つ右にシフトして次の位置へ
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
