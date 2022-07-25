#pragma once
#include <SDL2/SDL.h>
#include "renderer/scene.hpp"

namespace fluidity
{

class FluidRenderer;

class GuiLayer
{
public:
    GuiLayer(SDL_Window* window, void* glContext, FluidRenderer* fluidRenderer);
    bool Init();
    bool ProcessEvent(const SDL_Event& e);
    void Render();

private:
    void GuiLayer::RenderPerformanceOverlay();

    bool m_showPerformanceOverlay;

    SDL_Window* m_window;
    void* m_glContext;
    FluidRenderer* m_fluidRenderer;
    SceneSerializer m_sceneSerializer;
};

}