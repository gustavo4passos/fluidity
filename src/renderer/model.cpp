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
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        vertices.resize(assimpMesh->mNumVertices);

        for (int j = 0; j < assimpMesh->mNumVertices; j++)
        {
            const auto& vertex = assimpMesh->mVertices[j];
            vertices[j].position.x = vertex.x;
            vertices[j].position.y = vertex.y;
            vertices[j].position.z = vertex.z;

            // Same thing as calling HasNormals(), just avoid checking if mNumVertices > 0,
            // since if we are here, we already know it is.
            if (assimpMesh->mNormals != nullptr)
            {
                const auto& normal = assimpMesh->mNormals[j];
                vertices[j].normal.x = normal.x;
                vertices[j].normal.y = normal.y;
                vertices[j].normal.z = normal.z;
            }
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