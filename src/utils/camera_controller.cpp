#include "camera_controller.hpp"
#include <iostream>

namespace fluidity 
{
  CameraController::CameraController(const Camera& camera)
    : m_camera(camera),
    m_speed(1.f),
    m_movingOnAxis(0.f, 0.f, 0.f)
  { /* */ }

  void CameraController::ProcessInput(const SDL_Event& e)
  {
    if (e.type == SDL_KEYDOWN)
    {
      switch(e.key.keysym.sym)
      {
        // Move forward
        case SDLK_w:
        {
          m_movingOnAxis.z = 1.f;
          break;
        }
        case SDLK_s:
        {
          m_movingOnAxis.z = -1.f;
          break;
        }
        case SDLK_d:
        {
          m_movingOnAxis.x = 1.f;
          break;
        }
        case SDLK_a:
        {
          m_movingOnAxis.x = -1.f;
          break;
        }
        default: break;
      }
    }
    if (e.type == SDL_KEYUP)
    {
      switch(e.key.keysym.sym)
      {
        // Move forward
        case SDLK_w:
        {
          m_movingOnAxis.z = 0;
          break;
        }
        case SDLK_s:
        {
          m_movingOnAxis.z = 0;
          break;
        }
        case SDLK_d:
        {
          m_movingOnAxis.x = 0;
          break;
        }
        case SDLK_a:
        {
          m_movingOnAxis.x = 0;
          break;
        }
        default: break;
      }
    }
  }

  void CameraController::Update() 
  {
    auto right = glm::normalize(glm::cross(m_camera.GetFront(), Camera::UP));
    auto cameraPos = m_camera.GetPosition();
    cameraPos += m_movingOnAxis.x * m_speed * right;
    cameraPos += m_movingOnAxis.z * m_speed * m_camera.GetFront();
    m_camera.SetPosition(cameraPos);
  }
}


