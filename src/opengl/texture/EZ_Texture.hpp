#pragma once

#include <memory>

struct _EZ_Texture
{
    unsigned int texture;  // OpenGLテクスチャID
    int width;             // テクスチャの幅
    int height;            // テクスチャの高さ
    int channels;          // テクスチャのチャンネル数

    ~_EZ_Texture();
};

typedef std::shared_ptr<_EZ_Texture> EZ_Texture;

/// @brief テクスチャの作成
/// @param texture_file_path 画像ファイルパス
/// @return 作成されたテクスチャ
EZ_Texture EZ_CreateTexture(const char *texture_file_path);

/// @brief テクスチャの解放
void _EZ_DestroyTexture(_EZ_Texture *texture);

/// @brief テクスチャをバインド
/// @param texture OpenGLTexture構造体
/// @param slot テクスチャスロット (0, 1, 2...)
void _EZ_BindTexture(_EZ_Texture *texture, unsigned int slot);
