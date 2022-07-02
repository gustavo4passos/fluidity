#include "renderer/model.hpp"
#include "utils/logger.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <cassert>

Model::Model(const std::string& filePath)
    : m_filePath(filePath)
{ /* */ }

bool Model::Load()
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(m_filePath, aiProcess_Triangulate | 
        aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        LOG_ERROR("Unable to load model: [" + m_filePath + "] " + std::string(importer.GetErrorString()));
    }


    for (int i = 0; i < scene->mNumMeshes; i++)
    {
        auto* assimpMesh = scene->mMeshes[i];
        std::vector<vec3> vertices;
        std::vector<unsigned> indices;
        vertices.resize(assimpMesh->mNumVertices);

        for (int j = 0; j < assimpMesh->mNumVertices; j++)
        {
            const auto& vertex = assimpMesh->mVertices[j];
            vertices[0].x = vertex.x;
            vertices[0].y = vertex.y;
            vertices[0].z = vertex.z;
        }

        for (int j = 0; j < assimpMesh->mNumFaces; j++)
        {
            auto face = assimpMesh->mFaces[j];
            // Sanity check: Faces should be triangles
            assert(face.mNumIndices == 3);

            for (int k = 0; k < face.mNumIndices; k++)
            {
                indices.push_back(face.mIndices[k]);
            }
        }

        m_meshes.push_back(Mesh(vertices, indices));
        m_meshes.back().Init();
    }

    return true;
}