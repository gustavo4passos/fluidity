#pragma once
#include "camera.hpp"
#include <SDL2/SDL.h>

namespace fluidity
{
  class CameraController
  {
  public:
    CameraController(const Camera& camera);
    Camera& GetCamera() { return m_camera; } 

    void ProcessInput(const SDL_Event& e);
    void Update();

  private:
    void ProcessMouse();
    Camera m_camera;
    float m_speed;

    glm::vec2 m_lastMousePosition;
    float m_yaw;
    float m_pitch;

    glm::vec3 m_movingOnAxis;
  };
}
