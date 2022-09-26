#pragma once
#include "renderer/mesh.hpp"
#include <string>

class Model
{
public:
    Model(const std::string& filePath, bool genSmoothNormals = false);
    Model() = default;
    bool Load();
    void CleanUp();

    bool HasSmoothNormals() const { return m_genSmoothNormals; }
    // The model needs to be reloaded for this change to have effect
    void SetHasSmoothNormals(bool genSmoothNormals) { m_genSmoothNormals = genSmoothNormals; }

    std::vector<Mesh>& GetMeshes() { return m_meshes; }
    const std::string& GetFilePath() const { return m_filePath; }

    void SetPosition(const vec3& position) { m_position = position; }
    const vec3& GetPosition() const { return m_position; }

    const vec3& GetDiffuse() const { return m_diffuseColor; }
    void SetDiffuse(const vec3& diffuse) { m_diffuseColor = diffuse; }

    bool IsVisible() const { return m_isVisible; }
    void SetIsVisible(bool visible) { m_isVisible = visible; }

    bool GetHideFrontFaces() const { return m_hideFrontFaces; }
    void SetHideFrontFaces(bool hide) { m_hideFrontFaces = hide; }

private:
    vec3 m_position;
    vec3 m_diffuseColor   = { 0.4f, 0.4f, 0.4f };
    bool m_isVisible      = true;
    bool m_hideFrontFaces = false;

    std::string m_filePath;
    std::vector<Mesh> m_meshes;
    bool m_genSmoothNormals;
};