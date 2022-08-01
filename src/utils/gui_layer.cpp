#include "utils/gui_layer.hpp"
#include "renderer/fluid_renderer.hpp"
#include "tinyfiledialogs.h"
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>

namespace fluidity
{
GuiLayer::GuiLayer(SDL_Window* window, void* glContext, FluidRenderer* fluidRenderer)
    : m_window(window),
    m_glContext(glContext),
    m_fluidRenderer(fluidRenderer),
    m_showPerformanceOverlay(false),
    m_showParametersWindow(false),
    m_sceneSerializer("./defaults.yaml")
{ /* */ }

bool GuiLayer::Init()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(m_window, m_glContext);
    ImGui_ImplOpenGL3_Init("#version 450");

    return true;
}

bool GuiLayer::ProcessEvent(const SDL_Event& e)
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplSDL2_ProcessEvent(&e);

    if (io.WantCaptureMouse) return true;
    return false;
}

void GuiLayer::Render()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    // static bool showDemoWindow = true;
    // ImGui::ShowDemoWindow(&showDemoWindow);

    RenderMainMenuBar();

    if (m_showPerformanceOverlay) RenderPerformanceOverlay();
    if (m_showParametersWindow) RenderParametersWindow();    
    

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GuiLayer::RenderParametersWindow()
{
    if (ImGui::Begin("Parameters", &m_showParametersWindow))
    {
        if (ImGui::CollapsingHeader("Lighting"))
        {
            auto& light = m_fluidRenderer->m_scene.lights[0];
            ImGui::Text("Light");
            // ImGui::PushID((int)&light);
            ImGui::PushID(0);
            ImGui::DragFloat3("Position", (float*)&light.position, 0.5, -100.f, 100.f);
            ImGui::ColorEdit3("Diffuse", (float*)&light.diffuse);
            ImGui::ColorEdit3("Ambient", (float*)&light.ambient);
            ImGui::ColorEdit3("Specular", (float*)&light.specular);
            ImGui::PopID();

            auto& lightingParameters = m_fluidRenderer->m_scene.lightingParameters;
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::Checkbox("Render shadows", &lightingParameters.renderShadows);
            ImGui::SameLine();
            ImGui::Checkbox("PCF", &lightingParameters.usePcf);
            ImGui::Spacing();
            ImGui::DragFloat("Min Shadow Bias",  &lightingParameters.minShadowBias, 0.00001f, 0.f, 1.f);
            ImGui::DragFloat("Max Shadow Bias",  &lightingParameters.maxShadowBias, 0.00001f, 0.f, 1.f);
            ImGui::DragFloat("Shadow Intensity", &lightingParameters.shadowIntensity, 0.005f, 0.f, 1.f);
        }

        if (ImGui::CollapsingHeader("Fluid"))
        {
            auto& fluidParameters     = m_fluidRenderer->m_scene.fluidParameters;
            auto& material            = m_fluidRenderer->m_scene.fluidMaterial;
            auto& filteringParameters = m_fluidRenderer->m_scene.filteringParameters;

            ImGui::DragFloat("Attenuatiuon", (float*)&fluidParameters.attenuation, 0.005f, 0.f, 1.f);
            ImGui::DragFloat("Particle Radius", (float*)&fluidParameters.pointRadius, 0.0005, 0.0001);

            ImGui::Separator();
            ImGui::Text("Material");
            ImGui::ColorEdit3("Diffuse", (float*)&material.diffuse);
            ImGui::ColorEdit3("Ambient", (float*)&material.ambient);
            ImGui::ColorEdit3("Specular", (float*)&material.specular);
            ImGui::DragFloat("Shininess", (float*)&material.shininess, 0.5, 0, 1000);
            
            ImGui::Separator();
            ImGui::Checkbox("Transparent", &fluidParameters.transparentFluid);
            ImGui::SliderInt("Iterations", &filteringParameters.nIterations, 0, 20);
            ImGui::SliderInt("Filter Size", &filteringParameters.filterSize, 1, 30);
            ImGui::SliderInt("Max Filter Size", &filteringParameters.maxFilterSize, 1, 200);
            ImGui::Checkbox("Gamma Correction", &filteringParameters.gammaCorrection);
        }

        if (ImGui::CollapsingHeader("Background"))
        {
            auto& renderState = m_fluidRenderer->m_meshesPass->GetRenderState();
            ImGui::ColorEdit3("Background color", (float*)&renderState.clearColor);

            auto meshesPass       = m_fluidRenderer->m_meshesPass;
            auto meshesShadowPass = m_fluidRenderer->m_meshesShadowPass;

            ImGui::Separator();
            if (ImGui::Button("Load Skybox"))
            {
                LoadSkybox();
            }

            if (ImGui::Button("Clear Skybox"))
            {
                meshesPass->RemoveSkybox();
            }

            ImGui::Separator();
            ImGui::Text("Models");
            int count=0;
            for (auto& model : m_fluidRenderer->m_scene.models)
            {
                ImGui::Separator();
                ImGui::Spacing();
                ImGui::Text(model.GetFilePath().c_str());
                bool hasSmoothedNormals = model.HasSmoothNormals();
                // ImGui::PushID((int)&model);
                ImGui::PushID(count++);
                ImGui::Checkbox("Smooth Normals", &hasSmoothedNormals);
                if (hasSmoothedNormals != model.HasSmoothNormals())
                {
                    model.SetHasSmoothNormals(hasSmoothedNormals);
                    model.CleanUp();
                    model.Load();
                }
                ImGui::PopID();
            }

            if (ImGui::Button("Load Model"))
            {
                LoadModel();
            }
        }

        if (ImGui::CollapsingHeader("Camera"))
        {
            auto& camera = m_fluidRenderer->m_cameraController.GetCamera();
            auto positionGlm = camera.GetPosition();
            vec3 position = { positionGlm.x, positionGlm.y, positionGlm.z };
            ImGui::DragFloat3("Position", (float*)&position, 0.5f, -300.f, 300.f);
            camera.SetPosition({ position.x, position.y, position.z });

            float fov = camera.GetFOV();
            ImGui::DragFloat("FOV", &fov, 0.5, 1, 179);
            camera.SetFOV(fov);

            auto& cameraController = m_fluidRenderer->m_cameraController;
            float yaw   = cameraController.GetYaw();
            float pitch = cameraController.GetPitch();

            ImGui::DragFloat("Yaw", (float*)&yaw, 0.5, -100, 100);
            ImGui::DragFloat("Pitch", (float*)&pitch, 0.5, -100, 100);
        }

        if (ImGui::CollapsingHeader("Scene"))
        {
            if (ImGui::Button("Save scene"))
            {
                SaveScene();
            }

            if (ImGui::Button("Load Scene"))
            {
                LoadNewScene();
            }
        }
    }
    ImGui::End();
}

void GuiLayer::RenderPerformanceOverlay()
{
    static int corner = 0;
    ImGuiIO& io = ImGui::GetIO();
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | 
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    if (corner != -1)
    {
        const float PAD = 10.0f;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
        ImVec2 work_size = viewport->WorkSize;
        ImVec2 window_pos, window_pos_pivot;
        window_pos.x = (corner & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
        window_pos.y = (corner & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
        window_pos_pivot.x = (corner & 1) ? 1.0f : 0.0f;
        window_pos_pivot.y = (corner & 2) ? 1.0f : 0.0f;
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        window_flags |= ImGuiWindowFlags_NoMove;
    }
    ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
    if (ImGui::Begin("Performance Overlay", &m_showPerformanceOverlay, window_flags))
    {
        ImGui::Text("Performance\n" "(right-click to change position)");
        ImGui::Separator();
        ImGui::Text("%.1f FPS", io.Framerate);
        ImGui::Text("%.3f ms/frame", 1000.f / io.Framerate);

        if (ImGui::BeginPopupContextWindow())
        {
            if (ImGui::MenuItem("Custom",       NULL, corner == -1)) corner = -1;
            if (ImGui::MenuItem("Top-left",     NULL, corner == 0)) corner = 0;
            if (ImGui::MenuItem("Top-right",    NULL, corner == 1)) corner = 1;
            if (ImGui::MenuItem("Bottom-left",  NULL, corner == 2)) corner = 2;
            if (ImGui::MenuItem("Bottom-right", NULL, corner == 3)) corner = 3;
            if (m_showPerformanceOverlay && ImGui::MenuItem("Close")) m_showPerformanceOverlay = false;
            ImGui::EndPopup();
        }
    }
    
    ImGui::End();
}

void GuiLayer::RenderMainMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New Scene"))
            {
                m_fluidRenderer->SetScene(Scene::CreateEmptyScene());
                m_fluidRenderer->LoadScene();
            }

            if (ImGui::MenuItem("Load Scene"))
            {
                LoadNewScene();
            }

            ImGui::Separator();
            ImGui::Spacing();
            if (ImGui::MenuItem("Save Scene"))
            {

            }

            if (ImGui::MenuItem("Save Scene As"))
            {
                SaveScene();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Load"))
        {
            if (ImGui::MenuItem("Load Fluid"))
            {
               LoadFluid();
            }

            if (ImGui::MenuItem("Load Skybox"))
            {
                LoadSkybox();
            }

            if (ImGui::MenuItem("Load Model"))
            {
               LoadModel();
            }

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View"))
        {
            ImGui::MenuItem("Performance Overlay", nullptr, &m_showPerformanceOverlay);
            ImGui::MenuItem("Parameters Window", nullptr, &m_showParametersWindow);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void GuiLayer::LoadNewScene()
{
    const char* acceptedFileTypes[1] = { "*.yml" };
    auto exportScenePath = tinyfd_openFileDialog("Export Scene", nullptr, 1, acceptedFileTypes, "Scene files", 0);
    if (exportScenePath != nullptr)
    {
        SceneSerializer ss = SceneSerializer(exportScenePath);
        if (ss.Deserialize())
        {
            m_fluidRenderer->SetScene(ss.GetScene());
            m_fluidRenderer->LoadScene();
        }
    }
}

void GuiLayer::LoadFluid()
{
    const char* fileTypesAccepted[1] = { "*.npz" };
    const char* files = tinyfd_openFileDialog("Load Fluid", nullptr, 1, 
        fileTypesAccepted, ".npz files", true);
    
    std::vector<std::string> fileList;
    if (files != nullptr)
    {
        std::string filesStr = files;
        size_t pos = 0;
        size_t lastPos = 0;
        while((pos = filesStr.find('|', pos)) != std::string::npos)
        {
            fileList.push_back(filesStr.substr(lastPos, pos - lastPos));
            pos++;
            lastPos = pos;
        }
    }

    if (fileList.size() > 0)     
    {
        m_fluidRenderer->m_currentFrame = 0;
        m_fluidRenderer->m_scene.fluid.Load(fileList);
    }
}

void GuiLayer::SaveScene()
{
    const char* acceptedFileTypes[1] = { "*.yml" };
    auto exportScenePath = tinyfd_saveFileDialog("Export Scene", nullptr, 1, acceptedFileTypes, "Scene files");
    if (exportScenePath != nullptr) 
    {
        Scene sc = m_fluidRenderer->m_scene;
        sc.camera = m_fluidRenderer->m_cameraController.GetCamera();
        SceneSerializer ss = SceneSerializer(sc, exportScenePath);
        ss.Serialize();
    }
}

void GuiLayer::LoadModel()
{
    auto modelPath = tinyfd_openFileDialog("Export Scene", nullptr, 0, nullptr, "Model Files", 0);
    if (modelPath != nullptr)
    {
        Model m = Model(modelPath);
        if (m.Load())
        {
            m_fluidRenderer->m_scene.models.push_back(m);
        }
    }
}

void GuiLayer::LoadSkybox()
{
    auto skyboxPath = tinyfd_selectFolderDialog("Load Skybox", nullptr);
    if (skyboxPath != nullptr)
    {
        Skybox s = Skybox(skyboxPath);
        if (s.Init())
        {
            // TODO: The scene should provide the skybox automatically
            m_fluidRenderer->m_meshesPass->AddSkybox(s);
            m_fluidRenderer->m_scene.skyboxPath = skyboxPath;
        }
    }
}

}