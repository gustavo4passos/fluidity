#include "utils/gui_layer.hpp"
#include "utils/logger.h"
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
    m_showParametersWindow(false)
{ /* */ }

bool GuiLayer::Init()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(m_window, m_glContext);
    ImGui_ImplOpenGL3_Init("#version 450");

    SetDefaultThemeColors();

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
    RenderPlaybackBar();
    
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GuiLayer::RenderParametersWindow()
{
    if (ImGui::Begin("Parameters", &m_showParametersWindow))
    {
        if (ImGui::CollapsingHeader("Lighting"))
        {
            auto& lightingParameters = m_fluidRenderer->m_scene.lightingParameters;
            auto& light = m_fluidRenderer->m_scene.lights[0];
            ImGui::Text("Light");
            ImGui::Checkbox("Show Lights on Scene", &lightingParameters.showLightsOnScene);
            // TODO: The ## light identifier is used to avoid id conflicts on imgui, since it will create a hash
            // using the name of the window and the ## identifier.
            // However, it won't work when multiple lights are at play
            ImGui::DragFloat3("Position##light", (float*)&light.position, 0.5, -100.f, 100.f);
            ImGui::ColorEdit3("Diffuse##light", (float*)&light.diffuse);
            ImGui::ColorEdit3("Ambient##light", (float*)&light.ambient);
            ImGui::ColorEdit3("Specular##light", (float*)&light.specular);
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::Spacing();

            ImGui::Text("Shadows");
            ImGui::Checkbox("Render shadows", &lightingParameters.renderShadows);
            ImGui::SameLine();
            ImGui::Checkbox("PCF", &lightingParameters.usePcf);
            ImGui::Spacing();
            ImGui::DragFloat("Min Shadow Bias",  &lightingParameters.minShadowBias,   0.00001f, 0.f, 1.f);
            ImGui::DragFloat("Max Shadow Bias",  &lightingParameters.maxShadowBias,   0.00001f, 0.f, 1.f);
            ImGui::DragFloat("Shadow Intensity", &lightingParameters.shadowIntensity, 0.005f, 0.f, 1.f);

            ImGui::Spacing();
            ImGui::Spacing();
        }

        if (ImGui::CollapsingHeader("Fluid"))
        {
            auto& fluidParameters     = m_fluidRenderer->m_scene.fluidParameters;
            auto& material            = m_fluidRenderer->m_scene.fluidMaterial;
            auto& filteringParameters = m_fluidRenderer->m_scene.filteringParameters;

            ImGui::DragFloat("Attenuatiuon", (float*)&fluidParameters.attenuation, 0.005f, 0.f, 1.f);
            ImGui::DragFloat("Particle Radius", (float*)&fluidParameters.pointRadius, 0.0005, 0.0001);
            ImGui::DragFloat("Refraction Modifier", (float*)&fluidParameters.refractionModifier, 0.0005, 0.0001, 5, "%.4f");

            ImGui::Separator();
            ImGui::Text("Material");
            ImGui::ColorEdit3("Diffuse##fluidMaterial", (float*)&material.diffuse);
            ImGui::ColorEdit3("Ambient##fluidMaterial", (float*)&material.ambient);
            ImGui::ColorEdit3("Specular##fluidMaterial", (float*)&material.specular);
            ImGui::DragFloat("Shininess##fluidMaterial", (float*)&material.shininess, 0.5, 0, 1000);
            
            ImGui::Separator();
            ImGui::Checkbox("Transparent", &fluidParameters.transparentFluid);
            ImGui::SliderInt("Iterations", &filteringParameters.nIterations, 0, 20);
            ImGui::SliderInt("Filter Size", &filteringParameters.filterSize, 1, 30);
            ImGui::SliderInt("Max Filter Size", &filteringParameters.maxFilterSize, 1, 200);

            ImGui::Separator();
            ImGui::Checkbox("Gamma Correction", &filteringParameters.gammaCorrection);
            ImGui::Checkbox("Use Refactinon Mask", &filteringParameters.useRefractionMask);
        }

        if (ImGui::CollapsingHeader("Environment"))
        {
            auto& scene = m_fluidRenderer->m_scene;
            ImGui::ColorEdit3("Background color", (float*)&scene.clearColor);

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
            // Used to avoid id conflicts on imgui
            int modelIdHash = 0;
            for (auto& model : m_fluidRenderer->m_scene.models)
            {
                ImGui::Separator();
                ImGui::Spacing();
                ImGui::Text(model.GetFilePath().c_str());
                bool hasSmoothedNormals = model.HasSmoothNormals();
                ImGui::PushID(modelIdHash++);
                ImGui::Checkbox("Smooth Normals", &hasSmoothedNormals);
                if (hasSmoothedNormals != model.HasSmoothNormals())
                {
                    model.SetHasSmoothNormals(hasSmoothedNormals);
                    model.CleanUp();
                    model.Load();
                }

                ImGui::SameLine();
                bool visible = model.IsVisible();
                ImGui::Checkbox("Visible", &visible);
                model.SetIsVisible(visible);

                ImGui::SameLine();
                bool hideFrontFaces = model.GetHideFrontFaces();
                ImGui::Checkbox("Hide Front Faces", &hideFrontFaces);
                model.SetHideFrontFaces(hideFrontFaces);

                vec3 translation = model.GetTranslation();
                ImGui::DragFloat3("Translation", (float*)&translation, 0.05, -4000, 4000);
                model.SetTranslation(translation);
                ImGui::Spacing();

                vec3 scale = model.GetScale();
                float scale1f = scale.x;
                ImGui::DragFloat("Scale", &scale1f, 0.05, 0.05, 300);
                model.SetScale({ scale1f, scale1f, scale1f });

                auto& material = model.GetMaterial();
                ImGui::ColorEdit3("Diffuse", (float*)&material.diffuse);
                ImGui::ColorEdit3("Specular", (float*)&material.specular);
                ImGui::SliderFloat("Shininess", &material.shininess, 0.01, 3000);

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
        ImGui::Text("%d particles", m_fluidRenderer->m_scene.fluid.
            GetNumberOfParticles(m_fluidRenderer->GetCurrentFrame()));

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

void GuiLayer::RenderPlaybackBar()
{
    auto mousePos = ImGui::GetMousePos();

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | 
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove;
    ImGuiIO& io = ImGui::GetIO();
    auto windowPadding = ImGui::GetStyle().WindowPadding;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float PAD = 20.f;
    ImVec2 workSize = viewport->WorkSize;
    ImVec2 workPos  = viewport->WorkPos;
    ImVec2 windowPos, windowSize, windowPivot;
    windowPos.x = workPos.x;
    windowPos.y = viewport->Size.y;
    windowSize.x = workSize.x;
    windowPivot = { 0.f, 1.f };
    
    if (mousePos.y < workSize.y - (workSize.y * 0.2)) return;

    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, windowPivot);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

    ImGui::SetNextWindowBgAlpha(0.35f);
    if (ImGui::Begin("Playback Bar", nullptr, windowFlags))
    {
        auto textWidth = ImGui::CalcTextSize("Frame").x;
        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - textWidth) * 0.5);
        ImGui::Text("Playback");
        int currentFrame = m_fluidRenderer->GetCurrentFrame();
        int numberOfFrames = m_fluidRenderer->m_scene.fluid.GetNumberOfFrames();
        ImGui::SetNextItemWidth({ windowSize.x - windowPadding.x * 2 });
        ImGui::SliderInt("##playback-seek-bar", &currentFrame, 0, numberOfFrames, "%d");

        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - 100) * 0.5);
        if (ImGui::Button(m_fluidRenderer->IsPlaying() ? "Pause" : "Play", { 100, 30 }))
        {
            m_fluidRenderer->TogglePlayPause();
        }
        m_fluidRenderer->SetCurrentFrame(currentFrame);
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
                m_sceneSerializer = SceneSerializer();
            }

            if (ImGui::MenuItem("Load Scene"))
            {
                LoadNewScene();
            }

            ImGui::Separator();
            ImGui::Spacing();
            if (ImGui::MenuItem("Save Scene"))
            {
                SaveScene();
            }

            if (ImGui::MenuItem("Save Scene As"))
            {
                SaveSceneAs();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Load"))
        {
            if (ImGui::MenuItem("Load Fluid"))
            {
               LoadFluid();
            }

            if (ImGui::MenuItem("Clear Fluid"))
            {
               m_fluidRenderer->m_scene.fluid.CleanUp();
            }

            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::MenuItem("Load Skybox"))
            {
                LoadSkybox();
            }

            if (ImGui::MenuItem("Clear Skybox"))
            {
                m_fluidRenderer->m_meshesPass->RemoveSkybox();
            }

            ImGui::Separator();
            ImGui::Spacing();

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
        m_sceneSerializer = SceneSerializer(exportScenePath);
        if (m_sceneSerializer.Deserialize())
        {
            m_fluidRenderer->SetScene(m_sceneSerializer.GetScene());
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
        m_fluidRenderer->ResetPlayback();
        m_fluidRenderer->Play();
    }
}

std::string GuiLayer::GetNewSceneFilenameDialog()
{
    const char* acceptedFileTypes[1] = { "*.yml" };
    auto exportScenePath = tinyfd_saveFileDialog("Export Scene", nullptr, 1, acceptedFileTypes, "Scene files");
    if (exportScenePath != nullptr) 
    {
        // Add *.yml to the end of the file name, in case it already doesn't contain it
        std::string exportScenePathStr = exportScenePath;
        std::string sceneFileExtension = SceneSerializer::SCENE_FILE_EXTENSION;
        if (exportScenePathStr.size() < sceneFileExtension.size() + 1 || 
        !std::equal(
            sceneFileExtension.rbegin(), sceneFileExtension.rend(), 
            exportScenePathStr.rbegin()))
        {
            exportScenePathStr += sceneFileExtension;
        }

        return exportScenePathStr;
    }
    else return std::string();
}

void GuiLayer::SaveScene()
{
    if (m_sceneSerializer.GetFilePath().empty()) SaveSceneAs();
    else
    {
        m_sceneSerializer.SetScene(m_fluidRenderer->m_scene);
        m_sceneSerializer.Serialize();
    }
}

void GuiLayer::SaveSceneAs()
{
    auto sceneFileName = GetNewSceneFilenameDialog();
    if (!sceneFileName.empty()) 
    {
        std::cout << "rsrrs" << std::endl;
        Scene sc = m_fluidRenderer->m_scene;
        sc.camera = m_fluidRenderer->m_cameraController.GetCamera();
        m_sceneSerializer = SceneSerializer(sc, sceneFileName);
        m_sceneSerializer.Serialize();
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

void GuiLayer::SetDefaultThemeColors()
{
    ImGuiStyle* style = &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.156f, 0.156f, 0.156f, 1.f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.301f, 0.301f, 0.301f, 1.f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.301f, 0.301f, 0.301f, 1.f);
    colors[ImGuiCol_Border]                 = ImVec4(0.219f, 0.219f, 0.219f, 1.f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.27f, 0.27f, 0.27f, 1.f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.25f, 0.25f, 0.25f, 1.f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.21f, 0.21f, 0.21f, 1.f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.258f, 0.258f, 0.258f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.27f, 0.278, 0.27f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_Tab]                    = ImVec4(0.27f, 0.27f, 0.27f, 1.f);
    colors[ImGuiCol_TabActive]              = ImVec4(0.325f, 0.325f, 0.325f, 1.f);
    colors[ImGuiCol_TabHovered]             = ImVec4(0.309f, 0.309f, 0.309f, 1.f);
    colors[ImGuiCol_TabUnfocused]           = ImVec4(0.27f, 0.27f, 0.27f, 1.f);
    colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.325f, 0.325f, 0.325f, 1.f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.325f, 0.325f, 0.325f, 1.f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.27f, 0.27f, 0.27f, 1.f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(.9f, .9f, .9f, 1.f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(1.f, 1.f, 1.f, 1.f);
    colors[ImGuiCol_Button]                 = ImVec4(0.27f, 0.27f, 0.27f, 1.f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.23f, 0.23f, 0.23f, 1.f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.20f, 0.20f, 0.20f, 1.f);
    colors[ImGuiCol_Header]                 = ImVec4(0.258f, 0.258f, 0.258f, 1.00f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.25f, 0.25f, 0.25f, 1.f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.21f, 0.21f, 0.21f, 1.f);
    colors[ImGuiCol_Separator]              = colors[ImGuiCol_Border];
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.2f, 0.2f, 0.2f, 1.f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.18f, 0.18f, 0.18f, 1.f);
    colors[ImGuiCol_ResizeGrip]             = colors[ImGuiCol_Border];
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.2f, 0.2f, 0.2f, 1.f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.18f, 0.18f, 0.219f, 1.f);
    colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.f, 1.f, 0.f, 1.f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight]           = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

}