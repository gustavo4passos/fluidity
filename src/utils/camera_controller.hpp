#pragma once
#include "utils/camera.hpp"
#include "input/game_controller.hpp"
#include <SDL2/SDL.h>

namespace fluidity
{
  class CameraController
  {
  public:
    CameraController(const Camera& camera);
    Camera& GetCamera() { return m_camera; }
    void SetCamera(const Camera& camera) { m_camera = camera; }

    void ProcessInput(const SDL_Event& e);
    void Update();

  private:
    void ProcessMouse();
    // Assures pitches between -89 and 89 degrees
    void SetPitch(float pitch);
    
    Camera m_camera;
    float m_speed;

    glm::vec2 m_lastMousePosition;
    bool m_mouseClicked;

    glm::vec3 m_movingOnAxis;

    GameController m_gameController;
  };
}
