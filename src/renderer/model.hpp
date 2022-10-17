#pragma once
#include "renderer/mesh.hpp"
#include <string>

class Model
{
public:
    Model(const std::string& filePath, bool genSmoothNormals = false);
    Model();
    bool Load();
    void CleanUp();

    bool HasSmoothNormals() const { return m_genSmoothNormals; }
    // The model needs to be reloaded for this change to have effect
    void SetHasSmoothNormals(bool genSmoothNormals) { m_genSmoothNormals = genSmoothNormals; }

    std::vector<Mesh>& GetMeshes() { return m_meshes; }
    const std::string& GetFilePath() const { return m_filePath; }

    void SetTranslation(const vec3& translation) { m_translation = translation; }
    const vec3& GetTranslation() const { return m_translation; }

    bool IsVisible() const { return m_isVisible; }
    void SetIsVisible(bool visible) { m_isVisible = visible; }

    bool GetHideFrontFaces() const { return m_hideFrontFaces; }
    void SetHideFrontFaces(bool hide) { m_hideFrontFaces = hide; }

    // TODO: GetMaterialConst is needed when calling GetMaterial from a
    // const Material&. However, this is not very consistent with the rest
    // of the codebase (which usually uses Getters and Setters)
    Material& GetMaterial() { return m_material; }
    const Material& GetMaterialConst() const { return m_material; }
    
    const vec3& GetScale() const { return m_scale; }
    void SetScale(const vec3& scale) { m_scale = scale; }

    const vec3& GetRotation() const { return m_rotation; }
    void SetRotation(const vec3& rotation) { m_rotation = rotation; }

    glm::mat4 GetModelMatrix();

    void AddPlane();

private:
    Material m_material;
    bool m_isVisible      = true;
    bool m_hideFrontFaces = false;

    vec3 m_translation    = { 0.f, 0.f, 0.f };
    vec3 m_scale          = { 1.f, 1.f, 1.f };
    vec3 m_rotation       = { 0.f, 0.f, 0.f };

    std::string m_filePath;
    std::vector<Mesh> m_meshes;
    bool m_genSmoothNormals;
};
