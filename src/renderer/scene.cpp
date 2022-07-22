#include "renderer/scene.hpp"
#include <iostream>
#include <fstream>

namespace fluidity
{
YAML::Emitter& operator << (YAML::Emitter& out, const FilteringParameters& filteringParameters)
{
    const FilteringParameters& fp = filteringParameters;
    out << YAML::Flow;
    out << YAML::BeginSeq << fp.nIterations << fp.filterSize << fp.maxFilterSize << fp.gammaCorrection;
    out << YAML::EndSeq;

    return out;
}

YAML::Emitter& operator << (YAML::Emitter& out, const FluidRenderingParameters& fluidRenderingParameters)
{
    const FluidRenderingParameters& frp = fluidRenderingParameters;
    out << YAML::Flow;
    out << YAML::BeginSeq << frp.attenuation;
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

YAML::Emitter& operator << (YAML::Emitter& out, const ShadowMapParameters& shadowMapParameters)
{
    const ShadowMapParameters& smp = shadowMapParameters;
    out << YAML::Flow;
    out << YAML::BeginSeq << smp.minShadowBias << smp.maxShadowBias << smp.shadowIntensity << smp.shadowIntensity;
    out << YAML::EndSeq;

    return out;
}

YAML::Emitter& operator << (YAML::Emitter& out, const Camera& camera)
{
    using namespace YAML;
    auto position = camera.GetPosition();
    out << BeginMap;
        out << Key << "position" << Flow << BeginSeq << position.x << position.y << position.z << EndSeq;
        out << Key << "fov" << Value << camera.GetFOV();
    out << EndMap;

    return out;
}

SceneSerializer::SceneSerializer(const std::string& filePath)
    : m_filePath(filePath)
{ /* */ }

SceneSerializer::SceneSerializer(Scene* scene, const std::string& filePath)
    : m_filePath(filePath),
    m_scene(*scene)
{ /* */ }

void SceneSerializer::Serialize()
{  
    using namespace YAML;
    Emitter out;
    out << BeginMap;
        out << Key << "Version" << Value << SERIALIZER_VERSION;
        out << Key << "FilteringParameters" << m_scene.filteringParameters;
        out << Key << "FluidRenderingParameters" << m_scene.fluidRenderingParameters;
        out << Key << "ShadowMapParameters" << m_scene.shadowParameters;

        out << Key << "Lights";
        out << BeginSeq;
            for (const auto& l : m_scene.lights) out << l;
        out << EndSeq;
        out << Key << "Models" << BeginSeq;
            for (auto& m : m_scene.modelsPaths) out << m;
        out << EndSeq;
        out << Key << "Camera" << m_scene.camera;
        out << Key << "Skybox" << Value << m_scene.skyboxPath;

    out << EndMap;

    std::cout << out.c_str();
}

}