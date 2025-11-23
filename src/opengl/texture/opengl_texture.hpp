#pragma once

struct OpenGLTexture
{
    unsigned int texture;
    int width;
    int height;
    int channels;
};

/// @brief テクスチャの作成・読み込み
/// @param texture OpenGLTexture構造体
/// @param texture_file 画像ファイルパス
/// @return 成功したか
bool opengl_texture_init(OpenGLTexture *texture, const char *texture_file);

/// @brief テクスチャの解放
void opengl_texture_destroy(OpenGLTexture *texture);

/// @brief テクスチャをバインド
/// @param texture OpenGLTexture構造体
/// @param slot テクスチャスロット (0, 1, 2...)
void opengl_texture_bind(OpenGLTexture *texture, unsigned int slot);