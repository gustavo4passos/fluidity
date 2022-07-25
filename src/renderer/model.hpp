#pragma once
#include "renderer/mesh.hpp"
#include <string>

class Model
{
public:
    Model(const std::string& filePath, bool genSmoothNormals = false);
    bool Load();
    void CleanUp();

    bool HasSmoothNormals() const { return m_genSmoothNormals; }
    // The model needs to be reloaded for this change to have effect
    void SetHasSmoothNormals(bool genSmoothNormals) { m_genSmoothNormals = genSmoothNormals; }

    std::vector<Mesh>& GetMeshes() { return m_meshes; }
    const std::string& GetFilePath() const { return m_filePath; }

private:
    std::string m_filePath;
    std::vector<Mesh> m_meshes;
    bool m_genSmoothNormals;
};