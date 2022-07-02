#pragma once
#include "renderer/mesh.hpp"
#include <string>

class Model
{
public:
    Model(const std::string& filePath);
    bool Load();

    std::vector<Mesh>& GetMeshes() { return m_meshes; }

private:
    std::string m_filePath;
    std::vector<Mesh> m_meshes;
};