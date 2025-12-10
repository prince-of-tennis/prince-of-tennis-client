#pragma once
#include "glad/glad.h"

struct EZ_Mesh
{
    GLuint vao;                // Vertex Array Object
    GLuint vbo;                // Vertex Buffer Object
    GLuint ebo;                // Element Buffer Object
    unsigned int index_count;  // インデックスの数
};