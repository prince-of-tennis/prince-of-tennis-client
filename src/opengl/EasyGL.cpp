#include "opengl/EasyGL.hpp"

#include <SDL2/SDL.h>

#include "opengl/2d/EZ_2d.h"
#include "opengl/glad/glad.h"
#include "util/log.hpp"

bool EZ_Init(EasyGL *gl, Context *context)
{
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    gl->context = SDL_GL_CreateContext(context->window);
    if (gl->context == NULL)
    {
        LOG_ERROR("GL_Contextに失敗しました: " << SDL_GetError());
        return false;
    }

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        std::cerr << "GLADの初期化に失敗しました。" << std::endl;
        SDL_GL_DeleteContext(gl->context);
        return false;
    }

    glViewport(0, 0, context->window_width, context->window_height);

    LOG_DEBUG("OpenGL Version: " << glGetString(GL_VERSION));

    glEnable(GL_DEPTH_TEST);

    return true;
}

EasyGL::~EasyGL()
{
    _EZ_2D_Destroy();
}
