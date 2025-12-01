#pragma once
#include "glad/glad.h"

struct OpenGLMesh
{
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    unsigned int index_count;
};