#pragma once

#include <SDL2/SDL.h>

#include <memory>

/// @brief SDLの構造体の除去用のヘルパー
/// @tparam T 型
template <typename T>
struct SdlDeleter;

/// @brief SDL_Windowの除去
template <>
struct SdlDeleter<SDL_Window>
{
    void operator()(SDL_Window *w) const
    {
        SDL_DestroyWindow(w);
    }
};

/// @brief SDL_Rendererの除去
template <>
struct SdlDeleter<SDL_Renderer>
{
    void operator()(SDL_Renderer *r) const
    {
        SDL_DestroyRenderer(r);
    }
};

/// @brief SDL_Surfaceの除去
template <>
struct SdlDeleter<SDL_Surface>
{
    void operator()(SDL_Surface *s) const
    {
        SDL_FreeSurface(s);
    }
};

/// @brief SDL_Textureの除去
template <>
struct SdlDeleter<SDL_Texture>
{
    void operator()(SDL_Texture *t) const
    {
        SDL_DestroyTexture(t);
    }
};

/// @brief unique_ptrのヘルパー
/// @tparam T 型
template <typename T>
using SdlPtr = std::unique_ptr<T, SdlDeleter<T>>;