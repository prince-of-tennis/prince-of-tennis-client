#include "EZ_Model.hpp"

#include <SDL2/SDL.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <assimp/Importer.hpp>
#include <vector>

#include "glad/glad.h"
#include "util/log.hpp"

using namespace std;

EZ_Model EZ_CreateModel(string model_file_path)
{
    auto model = make_shared<_EZ_Model>();
    LOG_DEBUG("モデル読み込み開始: " << model_file_path);

    Assimp::Importer importer;
    model->model_file_path = model_file_path;
    const aiScene *scene =
        importer.ReadFile(model->model_file_path.c_str(),
                          aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        LOG_ERROR("Assimpモデル読み込み失敗: " << importer.GetErrorString());
        return nullptr;
    }
    LOG_SUCCESS("Assimpモデル読み込み成功: meshes=" << scene->mNumMeshes);

    for (int i = 0; i < scene->mNumMeshes; i++)
    {
        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        const aiMesh *mesh = scene->mMeshes[i];

        for (unsigned int j = 0; j < mesh->mNumVertices; j++)
        {
            vertices.push_back(mesh->mVertices[j].x);
            vertices.push_back(mesh->mVertices[j].y);
            vertices.push_back(mesh->mVertices[j].z);

            if (mesh->mTextureCoords[0])
            {
                vertices.push_back(mesh->mTextureCoords[0][j].x);
                vertices.push_back(mesh->mTextureCoords[0][j].y);
            }
            else
            {
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
            }

            vertices.push_back(mesh->mNormals[j].x);
            vertices.push_back(mesh->mNormals[j].y);
            vertices.push_back(mesh->mNormals[j].z);
        }
        for (unsigned int j = 0; j < mesh->mNumFaces; j++)
        {
            aiFace face = mesh->mFaces[j];
            for (unsigned int k = 0; k < face.mNumIndices; k++)
            {
                indices.push_back(face.mIndices[k]);
            }
        }

        EZ_Mesh ez_mesh;

        ez_mesh.index_count = indices.size();
        LOG_DEBUG("モデルデータ: vertices=" << vertices.size() / 8
                                            << ", indices=" << indices.size());

        glGenVertexArrays(1, &ez_mesh.vao);
        glGenBuffers(1, &ez_mesh.vbo);
        glGenBuffers(1, &ez_mesh.ebo);
        LOG_DEBUG("OpenGL バッファ生成: VAO=" << ez_mesh.vao << ", VBO=" << ez_mesh.vbo
                                              << ", EBO=" << ez_mesh.ebo);

        glBindVertexArray(ez_mesh.vao);

        glBindBuffer(GL_ARRAY_BUFFER, ez_mesh.vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(),
                     GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ez_mesh.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(),
                     GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                              (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                              (void *)(5 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        model->meshes.push_back(ez_mesh);
    }
    return model;
}

void _EZ_DestroyModel(_EZ_Model *model)
{
    for (auto mesh : model->meshes)
    {
        if (mesh.vao != 0)
        {
            glDeleteVertexArrays(1, &mesh.vao);
            mesh.vao = 0;
        }

        if (mesh.vbo != 0)
        {
            glDeleteBuffers(1, &mesh.vbo);
            mesh.vbo = 0;
        }

        if (mesh.ebo != 0)
        {
            glDeleteBuffers(1, &mesh.ebo);
            mesh.ebo = 0;
        }
    }

    model->meshes.clear();
}

void _EZ_DrawModel(_EZ_Model *model)
{
    for (auto mesh : model->meshes)
    {
        glBindVertexArray(mesh.vao);
        glDrawElements(GL_TRIANGLES, mesh.index_count, GL_UNSIGNED_INT, 0);
    }
    glBindVertexArray(0);
}

_EZ_Model::~_EZ_Model()
{
    _EZ_DestroyModel(this);
}