#pragma once
#include "renderer/rendering_parameters.hpp"
#include "utils/camera.hpp"
#include "vec.hpp"
#include <vector>
#include <string>
#include <yaml-cpp/yaml.h>

namespace fluidity
{
struct Scene
{
    FilteringParameters filteringParameters;
    FluidRenderingParameters fluidRenderingParameters;
    ShadowMapParameters shadowParameters;
    Camera camera;
    std::vector<PointLight> lights;
    std::vector<std::string> modelsPaths;
    std::string skyboxPath;
};


class SceneSerializer
{
public:
    SceneSerializer(const std::string& filePath);
    SceneSerializer(Scene* scene, const std::string& filePath);
    void Deserialize();
    void Serialize();

    static constexpr char* SERIALIZER_VERSION = "1.0";

private:
    Scene m_scene;
    std::string m_filePath;
};
}