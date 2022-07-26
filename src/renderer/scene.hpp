#pragma once
#include "renderer/model.hpp"
#include "renderer/skybox.hpp"
#include "renderer/rendering_parameters.hpp"
#include "utils/camera.hpp"
#include "Fluid.hpp"
#include "vec.hpp"
#include <vector>
#include <string>
#include <yaml-cpp/yaml.h>

namespace fluidity
{
struct Scene
{
    Fluid fluid;
    FilteringParameters filteringParameters;
    FluidParameters fluidParameters;
    LightingParameters lightingParameters;
    Camera camera;
    Material fluidMaterial;
    std::vector<PointLight> lights = {};
    std::vector<Model> models;
    std::string skyboxPath;

    static Scene CreateEmptyScene() 
    {
        return {
            {},
            { 4, 7, 100, false },
            { 0.25, true, 0.0625 },
            { 0.00100000005, 0.00999999978, 0.5, true, true },
            { { 17, 8, 0.5 }},
            {
                { 0.2, 0.2, 0.2, 0.2 },
                { 1, 1, 1, 1 },
                { 1, 1, 1, 1 },
                350.f,
            }
        };
    }
};


class SceneSerializer
{
public:
    SceneSerializer(const std::string& filePath);
    SceneSerializer(Scene scene, const std::string& filePath);
    
    bool Deserialize();
    void Serialize();
    void SetScene(const Scene& scene) { m_scene = scene; }
    const Scene& GetScene() { return m_scene; }

    static constexpr char* SERIALIZER_VERSION = "1.0";

private:
    Scene m_scene;
    std::string m_filePath;
};
}
