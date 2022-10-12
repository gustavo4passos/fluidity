#pragma once
#include "renderer/model.hpp"
#include "renderer/skybox.hpp"
#include "renderer/rendering_parameters.hpp"
#include "renderer/npz_fluid.hpp"
#include "simulation/simulated_fluid.hpp"
#include "utils/camera.hpp"
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
    UbMaterial fluidMaterial;
    std::vector<PointLight> lights = {};
    std::vector<Model> models;
    std::string skyboxPath;
    Vec4 clearColor; 
    bool useSimulatedFluid = false;
    FluidSimulationParameters fluidSimulationParameters;

    static Scene CreateEmptyScene() 
    {
        return {
            {}, // Fluid
            { 4, 7, 100, false, true, false }, // Filtering parameters
            { 0.25, true, 0.0625, 1.0, false, 0.0 }, // Fluid parameters
            { 0.00100000005, 0.00999999978, 0.5, true, true }, // Lighting paremeters
            { Camera::GetDefaultPosition() }, // Camera
            {
                { 0.2, 0.2, 0.2, 0.2 },
                { 1, 1, 1, 1 },
                { 1, 1, 1, 1 },
                350.f,
            }, // Material
            { }, // Lights
            { }, // Models
            { }, // Skybox path
            { .5f, .5f, .5f, 1.f }
        };
    }
    
    void AddPlane(const vec3& translation = { 0.f, 0.f, 0.f }, 
        float scale = 1.f)
    {
      Model m;
      m.AddPlane();
      m.SetScale({ scale, scale, scale });
      m.SetTranslation(translation);
      models.push_back(m);
    }
};


class SceneSerializer
{
public:
    SceneSerializer() = default;
    SceneSerializer(const std::string& filePath);
    SceneSerializer(Scene scene, const std::string& filePath);

    bool Deserialize();
    void Serialize();
    void SetScene(const Scene& scene) { m_scene = scene; }
    const Scene& GetScene() { return m_scene; }

    const std::string& GetFilePath() const { return m_filePath; }
    void SetFilePath(const std::string filePath) { m_filePath = filePath; }

    static constexpr char* SERIALIZER_VERSION = "1.1";
    static constexpr char* SCENE_FILE_EXTENSION = ".yml";

private:
    Scene m_scene;
    std::string m_filePath;

    void SerializeModel(YAML::Emitter& out, const Model& m);
    void SerializeNPZFluid(YAML::Emitter& out, const Fluid& f);
    void SerializeSimulatedFluid(YAML::Emitter& out, const FluidSimulationParameters& p);

    bool DeserializeNPZFluid(const YAML::Node& node, Fluid& f);
    bool DeserializeModel(const YAML::Node& node, Model& m);

    std::string GetRelativePathFromSceneFile(const std::string& path);
    std::string GetAbsolutePathRelativeToScene(const std::string path);
};
}
