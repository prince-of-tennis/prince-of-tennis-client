#pragma once

#include <SDL2/SDL.h>

#include <memory>

#include "opengl/object/opengl_object.hpp"

/// @brief SDLの構造体の除去用のヘルパー
/// @tparam T 型
template <typename T>
struct Deleter;

/// @brief SDL_Windowの除去
template <>
struct Deleter<SDL_Window>
{
    void operator()(SDL_Window *w) const
    {
        SDL_DestroyWindow(w);
    }
};

/// @brief SDL_Rendererの除去
template <>
struct Deleter<SDL_Renderer>
{
    void operator()(SDL_Renderer *r) const
    {
        SDL_DestroyRenderer(r);
    }
};

/// @brief SDL_Surfaceの除去
template <>
struct Deleter<SDL_Surface>
{
    void operator()(SDL_Surface *s) const
    {
        SDL_FreeSurface(s);
    }
};

/// @brief SDL_Textureの除去
template <>
struct Deleter<SDL_Texture>
{
    void operator()(SDL_Texture *t) const
    {
        SDL_DestroyTexture(t);
    }
};

/// @brief OpenGLObjectの除去
template <>
struct Deleter<OpenGLObject>
{
    void operator()(OpenGLObject *o) const
    {
        opengl_object_destroy(o);
    }
};

/// @brief unique_ptrのヘルパー
/// @tparam T 型
template <typename T>
using UniquePtr = std::unique_ptr<T, Deleter<T>>;