#pragma once
#include "glad/glad.h"

struct EZ_Mesh
{
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    unsigned int index_count;
};