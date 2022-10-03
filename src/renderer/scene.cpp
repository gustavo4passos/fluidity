#include "renderer/scene.hpp"
#include "utils/logger.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

template<>
struct YAML::convert<fluidity::FilteringParameters>
{
    static bool decode(const YAML::Node& node, fluidity::FilteringParameters& fp)
    {
        if (!node.IsSequence() || node.size() < 5) return false;

        fp.nIterations       = node[0].as<int>();
        fp.filterSize        = node[1].as<int>();
        fp.maxFilterSize     = node[2].as<int>();
        fp.gammaCorrection   = node[3].as<bool>();
        fp.useRefractionMask = node[4].as<bool>();
        if (node.size() > 5) fp.filter1D = node[5].as<bool>();

        return true;
    }
};

template<>
struct YAML::convert<fluidity::FluidParameters>
{
    static bool decode(const YAML::Node& node, fluidity::FluidParameters& fp)
    {
        if (!node.IsSequence() || node.size() != 3) return false;

        fp.attenuation       = node[0].as<float>();
        fp.transparentFluid  = node[1].as<bool>();
        fp.pointRadius       = node[2].as<float>();
        return true;
    }
};

template<>
struct YAML::convert<fluidity::LightingParameters>
{
    static bool decode(const YAML::Node& node, fluidity::LightingParameters& lp)
    {
        if (!node.IsSequence() || node.size() < 5) return false;

        lp.minShadowBias     = node[0].as<float>();
        lp.maxShadowBias     = node[1].as<float>();
        lp.shadowIntensity   = node[2].as<float>();
        lp.usePcf            = node[3].as<bool>();
        lp.renderShadows     = node[4].as<bool>();
        
        if (node.size() > 5) 
        {
            lp.showLightsOnScene = node[5].as<bool>();
        }

        if (node.size() > 6) 
        {
            lp.renderFluidShadows = node[6].as<bool>();
        }

        if (node.size() > 7)
        {
            lp.fluidShadowIntensity = node[7].as<float>();
        }
        return true;
    }
};

template<>
struct YAML::convert<Vec4>
{
    static bool decode(const YAML::Node& node, Vec4& v)
    {
        if (!node.IsSequence() || node.size() != 4) return false;

        v.x = node[0].as<float>();
        v.y = node[1].as<float>();
        v.z = node[2].as<float>();
        v.w = node[3].as<float>();
        return true;
    }
};

template<>
struct YAML::convert<vec3>
{
    static bool decode(const YAML::Node& node, vec3& v)
    {
        if (!node.IsSequence() || node.size() != 3) return false;

        v.x = node[0].as<float>();
        v.y = node[1].as<float>();
        v.z = node[2].as<float>();
        return true;
    }
};

template<>
struct YAML::convert<glm::vec3>
{
    static bool decode(const YAML::Node& node, glm::vec3& v)
    {
        if (!node.IsSequence() || node.size() != 3) return false;

        v.x = node[0].as<float>();
        v.y = node[1].as<float>();
        v.z = node[2].as<float>();
        return true;
    }
};


template<>
struct YAML::convert<PointLight>
{
    static bool decode(const YAML::Node& node, PointLight& l)
    {
        if (!node.IsMap()) return false;

        l.ambient  = node["ambient"].as<Vec4>();
        l.diffuse  = node["diffuse"].as<Vec4>();
        l.specular = node["specular"].as<Vec4>();
        l.position = node["position"].as<Vec4>();
        return true;
    }
};

template<>
struct YAML::convert<UbMaterial>
{
    static bool decode(const YAML::Node& node, UbMaterial& m)
    {
        if (!node.IsMap()) return false;

        m.ambient   = node["ambient"].as<Vec4>();
        m.diffuse   = node["diffuse"].as<Vec4>();
        m.specular  = node["specular"].as<Vec4>();
        m.shininess = node["shininess"].as<float>();
        return true;
    }
};

template<>
struct YAML::convert<Material>
{
    static bool decode(const YAML::Node& node, Material& m)
    {
        if (!node.IsMap()) return false;

        m.ambient   = node["ambient"].as<vec3>();
        m.diffuse   = node["diffuse"].as<vec3>();
        m.specular  = node["specular"].as<vec3>();
        m.shininess = node["shininess"].as<float>();
        m.emissive  = node["emissive"].as<bool>();

        if (node["reflectiveness"]) m.reflectiveness = node["reflectiveness"].as<float>();
        else m.reflectiveness = 0;
        
        return true;
    }
};

template<>
struct YAML::convert<fluidity::Camera>
{
    static bool decode(const YAML::Node& node, fluidity::Camera& c)
    {
        if (!node.IsMap()) return false;

        auto position = node["position"].as<vec3>();
        c.SetPosition({ position.x, position.y, position.z });
        c.SetFOV(node["fov"].as<float>());

        if (node["yaw"]) c.SetYaw(node["yaw"].as<float>());
        if (node["pitch"]) c.SetPitch(node["pitch"].as<float>());
        if (node["front"]) c.SetFront(node["front"].as<glm::vec3>());

        return true;
    }
};

namespace fluidity
{
YAML::Emitter& operator << (YAML::Emitter& out, const FilteringParameters& filteringParameters)
{
    const FilteringParameters& fp = filteringParameters;
    out << YAML::Flow;
    out << YAML::BeginSeq << fp.nIterations << fp.filterSize << fp.maxFilterSize << 
        fp.gammaCorrection << fp.useRefractionMask << fp.filter1D;
    out << YAML::EndSeq;

    return out;
}

YAML::Emitter& operator << (YAML::Emitter& out, const FluidParameters& fluidParameters)
{
    const FluidParameters& fp = fluidParameters;
    out << YAML::Flow;
    out << YAML::BeginSeq << fp.attenuation << fp.transparentFluid << fp.pointRadius;
    out << YAML::EndSeq;

    return out;
}

YAML::Emitter& operator << (YAML::Emitter& out, const Vec4& vec)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << vec.x << vec.y << vec.z << vec.w;
    out << YAML::EndSeq;

    return out;
}

YAML::Emitter& operator << (YAML::Emitter& out, const vec3& vec)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << vec.x << vec.y << vec.z;
    out << YAML::EndSeq;

    return out;
}

YAML::Emitter& operator << (YAML::Emitter& out, const glm::vec3& vec)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << vec.x << vec.y << vec.z;
    out << YAML::EndSeq;

    return out;
}

YAML::Emitter& operator << (YAML::Emitter& out, const LightingParameters& lightingParameters)
{
    const LightingParameters& lp = lightingParameters; 
    out << YAML::Flow;
    out << YAML::BeginSeq << lp.minShadowBias << lp.maxShadowBias << lp.shadowIntensity 
        << lp.usePcf << lp.renderShadows << lp.showLightsOnScene << lp.renderFluidShadows
        << lp.fluidShadowIntensity;
    out << YAML::EndSeq;

    return out;
}

YAML::Emitter& operator << (YAML::Emitter& out, const PointLight& light)
{
    using namespace YAML;
    out << BeginMap;
        out << Key << "ambient" << light.ambient;
        out << Key << "diffuse" << light.diffuse;
        out << Key << "specular" << light.specular;
        out << Key << "position" << light.position;
    out << EndMap;
    return out;
}

YAML::Emitter& operator << (YAML::Emitter& out, const UbMaterial& material)
{
    using namespace YAML;
    out << BeginMap;
        out << Key << "ambient"   << material.ambient;
        out << Key << "diffuse"   << material.diffuse;
        out << Key << "specular"  << material.specular;
        out << Key << "shininess" << material.shininess;
    out << EndMap;
    return out;
}

YAML::Emitter& operator << (YAML::Emitter& out, const Material& material)
{
    using namespace YAML;
    out << BeginMap;
        out << Key << "ambient"        << material.ambient;
        out << Key << "diffuse"        << material.diffuse;
        out << Key << "specular"       << material.specular;
        out << Key << "shininess"      << material.shininess;
        out << Key << "emissive"       << material.emissive;
        out << Key << "reflectiveness" << material.reflectiveness;
    out << EndMap;
    return out;
}

YAML::Emitter& operator << (YAML::Emitter& out, const Camera& camera)
{
    using namespace YAML;
    auto position = camera.GetPosition();
    out << BeginMap;
        out << Key << "position" << Flow << BeginSeq << position.x << position.y << position.z << EndSeq;
        out << Key << "fov" << Value << camera.GetFOV();
        out << Key << "yaw" << Value <<  camera.GetYaw();
        out << Key << "pitch" << Value << camera.GetPitch();
        out << Key << "front" << Value << camera.GetFront();
    out << EndMap;

    return out;
}

SceneSerializer::SceneSerializer(const std::string& filePath)
    : m_filePath(filePath)
{ /* */ }

SceneSerializer::SceneSerializer(Scene scene, const std::string& filePath)
    : m_filePath(filePath),
    m_scene(scene)
{ /* */ }

void SceneSerializer::Serialize()
{  
    std::filesystem::path basePath(m_filePath);

    if (!basePath.has_filename())
    {
        LOG_ERROR("Scene file name cannot be a directory.");
        return;
    }

    using namespace YAML;
    Emitter out;
    out << BeginMap;
        out << Key << "Version"    << Value << SERIALIZER_VERSION;
        out << Key << "ClearColor" << Value << m_scene.clearColor;
        out << Key << "FilteringParameters" << m_scene.filteringParameters;
        out << Key << "FluidParameters"     << m_scene.fluidParameters;
        out << Key << "LightingParameters"  << m_scene.lightingParameters;
        out << Key << "FluidMaterial"       << m_scene.fluidMaterial;

        out << Key << "Lights";
        out << BeginSeq;
            for (const auto& l : m_scene.lights) out << l;
        out << EndSeq;
        out << Key << "Models" << BeginSeq;
            for (auto& m : m_scene.models) SerializeModel(out, m);
        out << EndSeq;
        out << Key << "Camera" << m_scene.camera;
        out << Key << "Skybox" << Value << GetRelativePathFromSceneFile(m_scene.skyboxPath);
        SerializeFluid(out, m_scene.fluid);

    out << EndMap;

    std::ofstream outputFile(m_filePath);
    outputFile << out.c_str();
}

bool SceneSerializer::Deserialize()
{
    std::ifstream sceneFile(m_filePath);

    if (!sceneFile)
    {
        LOG_ERROR("Unable to open scene file: " + m_filePath);
        return false;
    }
    std::stringstream contentStream;
    contentStream << sceneFile.rdbuf();

    YAML::Node root = YAML::Load(contentStream.str());

    Scene sc = Scene::CreateEmptyScene();
    if (root["FilteringParameters"])
    {
        sc.filteringParameters = root["FilteringParameters"].as<FilteringParameters>();
    }

    if (root["FluidParameters"])
    {
        sc.fluidParameters = root["FluidParameters"].as<FluidParameters>();
    }

    if (root["LightingParameters"])
    {
        sc.lightingParameters = root["LightingParameters"].as<LightingParameters>();
    }

    if (root["Lights"] && root["Lights"].IsSequence())
    {
        for (const auto& l : root["Lights"])
        {
            sc.lights.push_back(l.as<PointLight>());
        }
    }

    if (root["FluidMaterial"])
    {
        sc.fluidMaterial = root["FluidMaterial"].as<UbMaterial>();
    }

    if (root["Camera"])
    {
        sc.camera = root["Camera"].as<Camera>();
    }

    if (root["Models"] && root["Models"].IsSequence())
    {
        for (const auto& m : root["Models"])
        {
            Model preloadedModel;
            // If model didn't deserialize properly, don't add it to scene
            if (!DeserializeModel(m, preloadedModel))
            {
                LOG_ERROR(m_filePath + ": Unable to load model " + preloadedModel.GetFilePath());
                continue;
            }

            if (preloadedModel.Load())
            {
                sc.models.push_back(preloadedModel);
            }
            else LOG_ERROR("Unable to load model: " + preloadedModel.GetFilePath());
        }
    }

    if (root["ClearColor"] && root["ClearColor"].IsSequence())
    {
        sc.clearColor = root["ClearColor"].as<Vec4>();
    }

    if (root["Skybox"])
    {
        sc.skyboxPath = GetAbsolutePathRelativeToScene(root["Skybox"].as<std::string>());
    }

    if (root["Fluid"])
    {
        if (!DeserializeFluid(root["Fluid"], sc.fluid))
        {
            LOG_ERROR(m_filePath + ": Unable to load fluid.");
        }

    }
    // TODO: Unecessary copy
    m_scene = sc;
    return true;
}


void SceneSerializer::SerializeModel(YAML::Emitter& out, const Model& m)
{
    using namespace YAML;
    out << BeginMap;
        out << Key << "filePath"         << Value << GetRelativePathFromSceneFile(m.GetFilePath());
        out << Key << "genSmoothNormals" << Value << m.HasSmoothNormals();
        out << Key << "material"         << Value << m.GetMaterialConst();
        out << Key << "hideFrontFaces"   << Value << m.GetHideFrontFaces();
        out << Key << "translation"      << Value << m.GetTranslation();
        out << Key << "scale"            << Value << m.GetScale();
        out << Key << "visible"          << Value << m.IsVisible();
    out << EndMap;
}

void SceneSerializer::SerializeFluid(YAML::Emitter& out, const Fluid& f)
{
    using namespace YAML;
    out << Key << "Fluid";
    out << BeginMap;
    out << Key << "type" << Value << "npz";
    out << Key << "fileList" << BeginSeq;

    for (const auto& f : f.GetFileList())
    {
        out << GetRelativePathFromSceneFile(f);
    }
    out << EndSeq << EndMap;
}

bool SceneSerializer::DeserializeFluid(const YAML::Node& node, Fluid& f)
{
    if (!node.IsMap()) return false;
    if (!node["fileList"] || !node["fileList"].IsSequence()) return false;

    // Absolute path is calculated her instead of using GetAbsolutePathRelativeToScene()
    // for optimization purposes. Since each call to GetAbsolutePathRelativeToScene changes 
    // the current path, and there can be many .npz files, we just do it adhoc, to avoid 
    // the multiple changes.
    std::filesystem::path oldPath = std::filesystem::current_path();
    std::filesystem::current_path(std::filesystem::path(m_filePath).parent_path());

    std::vector<std::string> fileList;
    for (const auto& f : node["fileList"])
    {
        std::filesystem::path absolutePath = std::filesystem::canonical(f.as<std::string>());
        fileList.push_back(absolutePath.generic_string());
    }
    std::filesystem::current_path(oldPath);

    if (fileList.size() > 0) f.Load(fileList);

    return true;
}


bool SceneSerializer::DeserializeModel(const YAML::Node& node, Model& m)
{
    if (!node.IsMap()) return false;

    auto filePath = GetAbsolutePathRelativeToScene(node["filePath"].as<std::string>());
    auto genSmoothNormals = node["genSmoothNormals"].as<bool>();
    m = Model(filePath, genSmoothNormals);

    if (node["material"])
    {
        auto material = node["material"].as<Material>();
        m.GetMaterial() = material;
    }

    if (node["hideFrontFaces"])
    {
        auto hideFrontFaces = node["hideFrontFaces"].as<bool>();
        m.SetHideFrontFaces(hideFrontFaces);
    }

    if (node["translation"])
    {
        auto translation = node["translation"].as<vec3>();
        m.SetTranslation(translation);
    }

    if (node["scale"])
    {
        auto scale = node["scale"].as<vec3>();
        m.SetScale(scale);
    }

    if (node["visible"])
    {
        auto visible = node["visible"].as<bool>();
        m.SetIsVisible(visible);
    }

    return true;
}


std::string SceneSerializer::GetRelativePathFromSceneFile(const std::string& path)
{
    std::filesystem::path scenePath(m_filePath);
    std::filesystem::path absolutePath(path);

    return std::filesystem::relative(absolutePath, scenePath.parent_path()).generic_string(); 
}

std::string SceneSerializer::GetAbsolutePathRelativeToScene(const std::string path)
{
    std::filesystem::path oldPath = std::filesystem::current_path();
    std::filesystem::current_path(std::filesystem::path(m_filePath).parent_path());
    std::filesystem::path absolutePath = std::filesystem::canonical(path);
    std::filesystem::current_path(oldPath);

    return absolutePath.generic_string(); 

}
}