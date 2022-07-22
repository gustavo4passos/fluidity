#include "utils/gui_layer.hpp"
#include "renderer/fluid_renderer.hpp"
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>

namespace fluidity
{
GuiLayer::GuiLayer(SDL_Window* window, void* glContext, FluidRenderer* fluidRenderer)
    : m_window(window),
    m_glContext(glContext),
    m_fluidRenderer(fluidRenderer)
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

    ImGui::Begin("Parameters");

    if (ImGui::CollapsingHeader("Lighting"))
    {
        auto& light = m_fluidRenderer->m_lights[0];
        ImGui::Text("Light");
        ImGui::PushID((int)&light);
        ImGui::DragFloat3("Position", (float*)&light.position, 0.5, -100.f, 100.f);
        ImGui::PopID();
        ImGui::ColorEdit3("Diffuse", (float*)&light.diffuse);
        ImGui::ColorEdit3("Ambient", (float*)&light.ambient);
        ImGui::ColorEdit3("Specular", (float*)&light.specular);
        ImGui::Separator();
        ImGui::Checkbox("Render shadows", &m_fluidRenderer->m_renderShadows);
    }

    if (ImGui::CollapsingHeader("Shadows"))
    {
        auto& shadowParameters = m_fluidRenderer->m_shadowMapParameters;
        ImGui::DragFloat("Min Shadow Bias",  &shadowParameters.minShadowBias, 0.00001f, 0.f, 1.f);
        ImGui::DragFloat("Max Shadow Bias",  &shadowParameters.maxShadowBias, 0.00001f, 0.f, 1.f);
        ImGui::DragFloat("Shadow Intensity", &shadowParameters.shadowIntensity, 0.05f, 0.f, 1.f);
        ImGui::Checkbox("PCF", &shadowParameters.usePcf);
    }

    if (ImGui::CollapsingHeader("Filtering"))
    {
        ImGui::Checkbox("Transparent", &m_fluidRenderer->m_transparentFluid);
        auto& filteringParameters = m_fluidRenderer->m_filteringParameters;
        ImGui::SliderInt("Iterations", &filteringParameters.nIterations, 0, 20);
        ImGui::SliderInt("Filter Size", &filteringParameters.filterSize, 1, 30);
        ImGui::SliderInt("Max Filter Size", &filteringParameters.maxFilterSize, 1, 200);
        ImGui::Checkbox("Gamma Correction", &filteringParameters.gammaCorrection);
    }

    if (ImGui::CollapsingHeader("Background"))
    {
        auto& renderState = m_fluidRenderer->m_meshesPass->GetRenderState();
        ImGui::ColorEdit3("Background color", (float*)&renderState.clearColor);
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

     if (ImGui::CollapsingHeader("Fluid"))
    {
        auto& fluidRenderingParameters = m_fluidRenderer->m_fluidRenderingParameters;
        ImGui::DragFloat("Attenuatiuon", (float*)&fluidRenderingParameters, 0.005f, 0.f, 1.f);
        ImGui::DragFloat("Particle Radius", (float*)&m_fluidRenderer->m_pointRadius, 0.0005, 0.0001);

        auto& material = m_fluidRenderer->m_fluidMaterial;
        ImGui::Separator();
        ImGui::Text("Material");
        ImGui::ColorEdit3("Diffuse", (float*)&material.diffuse);
        ImGui::ColorEdit3("Ambient", (float*)&material.ambient);
        ImGui::ColorEdit3("Specular", (float*)&material.specular);
        ImGui::DragFloat("Shininess", (float*)&material.shininess, 0.5, 0, 1000);

    }
    
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

}