#include "camera_controller.hpp"
#include <iostream>

namespace fluidity 
{
  CameraController::CameraController(const Camera& camera)
    : m_camera(camera),
    m_speed(.5f),
    m_movingOnAxis(0.f, 0.f, 0.f),
    m_yaw(0),
    m_pitch(0)
  { 
    int x, y;
    SDL_GetMouseState(&x, &y);
    m_lastMousePosition = glm::vec2(x, y);
  }

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

  void CameraController::ProcessMouse()
  {
    int x, y;
    SDL_GetMouseState(&x, &y);
    glm::vec2 mouseMove = m_lastMousePosition - glm::vec2(x, y); 
    m_lastMousePosition = glm::vec2(x, y);

    float sensitivity = .2f; 
    m_yaw   += mouseMove.x * sensitivity;
    m_pitch += mouseMove.y * sensitivity;

    if (m_pitch >  89.f) m_pitch = 89.f;
    if (m_pitch < -89.f) m_pitch = 89.f;
  }

  void CameraController::Update() 
  {
    ProcessMouse();
    glm::vec3 direction;
    direction.x = std::cos(glm::radians(m_yaw)) * std::cos(glm::radians(m_pitch));
    direction.y = std::sin(glm::radians(m_pitch));
    direction.z = std::sin(glm::radians(m_yaw)) * std::cos(glm::radians(m_pitch));

    glm::vec3 front = glm::normalize(direction); 
    std::cout << "(" << direction.x << ", " << direction.y << ", " << direction.z << ")\n";
    m_camera.SetFront(glm::normalize(direction));

    auto right = glm::normalize(glm::cross(m_camera.GetFront(), Camera::UP));
    auto cameraPos = m_camera.GetPosition();
    cameraPos += m_movingOnAxis.x * m_speed * right;
    cameraPos += m_movingOnAxis.z * m_speed * m_camera.GetFront();
    m_camera.SetPosition(cameraPos);
  }
}
