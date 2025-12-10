#pragma once

#include <SDL2/SDL.h>

#include <memory>

#include "network/network.hpp"
#include "opengl/object/EZ_Object.hpp"

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
struct Deleter<_EZ_Object>
{
    void operator()(_EZ_Object *o) const
    {
        _EZ_DestroyObject(o);
        delete o;
    }
};

template <>
struct Deleter<Network>
{
    void operator()(Network *network) const
    {
        network_fini(network);
        delete network;
    }
};

/// @brief unique_ptrのヘルパー
/// @tparam T 型
template <typename T>
using UniquePtr = std::unique_ptr<T, Deleter<T>>;