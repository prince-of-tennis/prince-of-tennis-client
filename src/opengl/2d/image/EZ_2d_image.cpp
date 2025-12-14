#include "EZ_2d_image.h"

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "stb_image.h"
#include "util/log.hpp"

using namespace std;

// 外部からアクセスできるグローバル変数（EZ_2d.cppから参照）
extern GLuint g_shader_program;
extern GLint g_uniform_projection;
extern GLint g_uniform_model;
extern GLint g_uniform_color;
extern GLint g_uniform_use_texture;
extern GLint g_uniform_is_text;
extern GLuint g_rect_vao;
extern glm::mat4 g_projection_matrix;

EZ_2D_Image EZ_2D_CreateImage(const char *image_path)
{
    auto image = make_shared<_EZ_2D_Image>();

    // stb_imageを使用して画像を読み込み
    stbi_set_flip_vertically_on_load(false);
    int channels;
    unsigned char *data = stbi_load(image_path, &image->width, &image->height, &channels, 0);

    if (!data)
    {
        LOG_ERROR("画像の読み込みに失敗: " << image_path);
        return nullptr;
    }

    // OpenGLテクスチャを生成
    glGenTextures(1, &image->texture_id);
    glBindTexture(GL_TEXTURE_2D, image->texture_id);

    // テクスチャパラメータを設定
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 画像データをテクスチャにアップロード
    GLenum format = GL_RGB;
    if (channels == 1)
        format = GL_RED;
    else if (channels == 3)
        format = GL_RGB;
    else if (channels == 4)
        format = GL_RGBA;

    glTexImage2D(GL_TEXTURE_2D, 0, format, image->width, image->height, 0, format, GL_UNSIGNED_BYTE,
                 data);

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);

    LOG_SUCCESS("画像読み込み完了: " << image_path << " (" << image->width << "x" << image->height
                                     << ", " << channels << " channels)");
    return image;
}

void _EZ_2D_DestroyImage(_EZ_2D_Image *image)
{
    if (!image)
    {
        return;
    }

    if (image->texture_id != 0)
    {
        glDeleteTextures(1, &image->texture_id);
        image->texture_id = 0;
    }

    LOG_SUCCESS("画像解放完了");
}

_EZ_2D_Image::~_EZ_2D_Image()
{
    _EZ_2D_DestroyImage(this);
}

void EZ_2D_DrawImage(EZ_2D_Image image, float x, float y, float width, float height, float r,
                     float g, float b, float a)
{
    if (!image || image->texture_id == 0)
    {
        LOG_ERROR("画像がNULLです。");
        return;
    }

    // 幅と高さが0の場合は元のサイズを使用
    if (width == 0) width = static_cast<float>(image->width);
    if (height == 0) height = static_cast<float>(image->height);

    glUseProgram(g_shader_program);

    // モデル行列: 位置とスケール
    auto model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x, y, 0.0f));
    model = glm::scale(model, glm::vec3(width, height, 1.0f));

    glUniformMatrix4fv(g_uniform_projection, 1, GL_FALSE, glm::value_ptr(g_projection_matrix));
    glUniformMatrix4fv(g_uniform_model, 1, GL_FALSE, glm::value_ptr(model));
    glUniform4f(g_uniform_color, r, g, b, a);
    glUniform1i(g_uniform_use_texture, 1);  // テクスチャを使用
    glUniform1i(g_uniform_is_text, 0);      // 画像描画（テキストではない）

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, image->texture_id);

    glBindVertexArray(g_rect_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);
}
