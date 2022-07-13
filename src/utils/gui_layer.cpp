#include "utils/gui_layer.hpp"
#include "renderer/fluid_renderer.h"
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
    static bool showDemoWindow = true;
    ImGui::ShowDemoWindow(&showDemoWindow);

    ImGui::Begin("Parameters");
    if (ImGui::CollapsingHeader("Lighting"))
    {
        auto& light = m_fluidRenderer->m_lights[0];
        ImGui::Text("Light");
        ImGui::DragFloat3("Position", (float*)&light.position, 0.5, -100.f, 100.f);
        ImGui::ColorEdit3("Diffuse", (float*)&light.diffuse);
        ImGui::ColorEdit3("Ambient", (float*)&light.ambient);
        ImGui::ColorEdit3("Specular", (float*)&light.specular);
        ImGui::Separator();
        ImGui::Checkbox("Render shadows", &m_fluidRenderer->m_renderShadows);
    }
    if (ImGui::CollapsingHeader("Filtering"))
    {
        ImGui::Checkbox("Transparent", &m_fluidRenderer->m_transparentFluid);
        auto& filteringParameters = m_fluidRenderer->m_filteringParameters;
        ImGui::SliderInt("Number of iterations", &filteringParameters.nIterations, 0, 20);
        ImGui::SliderInt("Filter Size", &filteringParameters.filterSize, 1, 30);
        ImGui::SliderInt("Max Filter Size", &filteringParameters.maxFilterSize, 1, 200);
    }

    if (ImGui::CollapsingHeader("Background"))
    {
        auto& renderState = m_fluidRenderer->m_meshesPass->GetRenderState();
        ImGui::ColorEdit3("Background color", (float*)&renderState.clearColor);
    }
    
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

}