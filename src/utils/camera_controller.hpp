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

    void ProcessInput(const SDL_Event& e);
    void Update();

    float GetYaw()   { return m_yaw;   };
    float GetPitch() { return m_pitch; };


  private:
    void ProcessMouse();
    // Assures pitches between -89 and 89 degrees
    void SetPitch(float pitch);
    
    Camera m_camera;
    float m_speed;

    glm::vec2 m_lastMousePosition;
    bool m_mouseClicked;
    float m_yaw;
    float m_pitch;

    glm::vec3 m_movingOnAxis;

    GameController m_gameController;
  };
}
