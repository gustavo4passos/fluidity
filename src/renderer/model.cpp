#include "renderer/model.hpp"
#include "utils/logger.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <cassert>

Model::Model(const std::string& filePath, bool genSmoothNormals)
    : m_filePath(filePath),
    m_genSmoothNormals(genSmoothNormals),
    m_material { 
        { 0.1f, 0.1f, 0.1f },
        { 0.4f, 0.4f, 0.4f },
        { 1.f,  1.f,  1.f },
        32.f,
        false
        }
{ /* */ }

bool Model::Load()
{
    Assimp::Importer importer;
    if (m_genSmoothNormals) importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, 
        aiComponent_NORMALS);

    const aiScene* scene = importer.ReadFile(m_filePath, aiProcess_Triangulate | aiProcess_FlipUVs | 
        (m_genSmoothNormals ? (aiProcess_RemoveComponent | aiProcess_GenSmoothNormals) : 0) |
        aiProcess_CalcTangentSpace);
    
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        LOG_ERROR("Unable to load model: [" + m_filePath + "] " + std::string(importer.GetErrorString()));
        return false;
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

void Model::CleanUp()
{
    for (auto& mesh : m_meshes)
    {
        mesh.CleanUp();
    }

    m_meshes.clear();
}