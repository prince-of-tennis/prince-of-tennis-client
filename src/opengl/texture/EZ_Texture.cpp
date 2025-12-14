#include "EZ_Texture.hpp"

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "glad/glad.h"
#include "stb_image.h"
#include "util/log.hpp"

using namespace std;

EZ_Texture EZ_CreateTexture(const char *texture_file_path)
{
    auto texture = make_shared<_EZ_Texture>();

    stbi_set_flip_vertically_on_load(true);

    unsigned char *data =
        stbi_load(texture_file_path, &texture->width, &texture->height, &texture->channels, 0);
    if (!data)
    {
        LOG_ERROR("テクスチャの読み込みに失敗しました: " << texture_file_path);
        return nullptr;
    }
    LOG_SUCCESS("画像読み込み成功: " << texture->width << "x" << texture->height
                                     << ", channels=" << texture->channels);

    glGenTextures(1, &texture->texture);
    glBindTexture(GL_TEXTURE_2D, texture->texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLenum format = GL_RGB;
    if (texture->channels == 1)
    {
        format = GL_RED;
    }
    else if (texture->channels == 3)
    {
        format = GL_RGB;
    }
    else if (texture->channels == 4)
    {
        format = GL_RGBA;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, format, texture->width, texture->height, 0, format,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    glBindTexture(GL_TEXTURE_2D, 0);

    LOG_SUCCESS("テクスチャ初期化完了: ID=" << texture->texture);
    return texture;
}

void _EZ_DestroyTexture(_EZ_Texture *texture)
{
    if (texture->texture != 0)
    {
        glDeleteTextures(1, &texture->texture);
        texture->texture = 0;
    }
}

void _EZ_BindTexture(_EZ_Texture *texture, unsigned int slot)
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, texture->texture);
}

_EZ_Texture::~_EZ_Texture()
{
    _EZ_DestroyTexture(this);
}