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
        false,
        0.f
        }
{ /* */ }

Model::Model()
    : m_filePath(""),
    m_genSmoothNormals(false),
    m_material { 
        { 0.1f, 0.1f, 0.1f },
        { 0.4f, 0.4f, 0.4f },
        { 1.f,  1.f,  1.f },
        32.f,
        false,
        0.f
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

glm::mat4 Model::GetModelMatrix()
{
    glm::mat4 modelMatrix = modelMatrix = glm::translate(glm::mat4(1.f), glm::vec3(m_translation));

    if (m_rotation.x != 0.f || m_rotation.y != 0.f || m_rotation.z != 0.f)
    {
      modelMatrix = glm::rotate(modelMatrix, m_rotation.x, glm::vec3(1.f, 0.f, 0.));
      modelMatrix = glm::rotate(modelMatrix, m_rotation.y, glm::vec3(0.f, 1.f, 0.));
      modelMatrix = glm::rotate(modelMatrix, m_rotation.z, glm::vec3(0.f, 0.f, 1.));
    }

    modelMatrix = glm::scale(modelMatrix, glm::vec3(m_scale));
    return modelMatrix;
}

void Model::AddPlane()
{
  std::vector<Vertex> vertices = {
    { { -0.5f, 0.f, -0.5f }, { 0.0, 1.f, 0.0 } },
    { {  0.5f, 0.f, -0.5f }, { 0.0, 1.f, 0.0 } },
    { {  0.5f, 0.f,  0.5f }, { 0.0, 1.f, 0.0 } },
    { { -0.5f, 0.f,  0.5f }, { 0.0, 1.f, 0.0 } }
  };                   

  std::vector<unsigned> indices = { 0, 1, 2, 0, 2, 3 };

  Mesh m(vertices, indices);
  m.Init();
  m_meshes.push_back(m);
}
