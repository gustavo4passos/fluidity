#include "camera_controller.hpp"
#include <iostream>

namespace fluidity 
{
  CameraController::CameraController(const Camera& camera)
    : m_camera(camera),
    m_speed(.3f),
    m_movingOnAxis(0.f, 0.f, 0.f),
    m_yaw(0),
    m_pitch(0),
    m_mouseClicked(false)
  { 
    // TODO: These should not be hardcoded
    m_yaw = 177;
    m_pitch = -21;

    int x, y;
    SDL_GetMouseState(&x, &y);
    m_lastMousePosition = glm::vec2(x, y);
    m_gameController.Init();
  }

  void CameraController::ProcessInput(const SDL_Event& e)
  {
    m_gameController.ProcessEvent(e);
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
        case SDLK_LSHIFT:
        {
          m_movingOnAxis.y = 1.f;
          break;
        }
        case SDLK_LCTRL:
        {
          m_movingOnAxis.y = -1.f;
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
        case SDLK_LSHIFT:
        {
          m_movingOnAxis.y = 0.f;
          break;
        }
        case SDLK_LCTRL:
        {
          m_movingOnAxis.y = 0.f;
          break;
        }
        default: break;
      }
    }

    if (e.type == SDL_MOUSEBUTTONDOWN)
    {
      if (e.button.button == SDL_BUTTON_LEFT)
      {
        m_mouseClicked = true;
        int x, y;
        SDL_GetMouseState(&x, &y);
        m_lastMousePosition = glm::vec2(x, y);
      }
    }

    if (e.type == SDL_MOUSEBUTTONUP)
    {
      if (e.button.button == SDL_BUTTON_LEFT)
      {
        m_mouseClicked = false;
      }
    }
  }

  void CameraController::ProcessMouse()
  {
    if (!m_mouseClicked) return;

    int x, y;
    SDL_GetMouseState(&x, &y);
    glm::vec2 mouseMove = m_lastMousePosition - glm::vec2(x, y); 
    m_lastMousePosition = glm::vec2(x, y);

    float sensitivity = .2f; 
    m_yaw   += mouseMove.x * sensitivity;
    SetPitch(m_pitch - mouseMove.y * sensitivity);
  }

  void CameraController::SetPitch(float pitch)
  {
    m_pitch = pitch;
    if (m_pitch >  89.f) m_pitch =  89.f;
    if (m_pitch < -89.f) m_pitch = -89.f;
  }

  void CameraController::Update() 
  {
    ProcessMouse();

#if GAME_CONTROLLER_ENABLED
    if (m_gameController.IsControllerConnected())
    {
      AxisStatus axisStatus = m_gameController.GetAxisStatus();

      m_yaw += axisStatus.rightAxis.x;
      SetPitch(m_pitch - axisStatus.rightAxis.y);

      m_movingOnAxis.x = axisStatus.leftAxis.x;
      m_movingOnAxis.z = -axisStatus.leftAxis.y;

      m_movingOnAxis.y = -axisStatus.triggers.x;
      if (axisStatus.triggers.y > 0) m_movingOnAxis.y = axisStatus.triggers.y;
    }
#endif

    glm::vec3 direction;
    direction.x = std::cos(glm::radians(m_yaw)) * std::cos(glm::radians(m_pitch));
    direction.y = std::sin(glm::radians(m_pitch));
    direction.z = std::sin(glm::radians(m_yaw)) * std::cos(glm::radians(m_pitch));

    glm::vec3 front = glm::normalize(direction); 
    m_camera.SetFront(glm::normalize(direction));

    auto right = glm::normalize(glm::cross(m_camera.GetFront(), Camera::UP));
    auto cameraPos = m_camera.GetPosition();
    cameraPos += m_movingOnAxis.x * m_speed * right;
    cameraPos += m_movingOnAxis.z * m_speed * m_camera.GetFront();
    cameraPos += m_movingOnAxis.y * m_speed * Camera::UP;
    m_camera.SetPosition(cameraPos);
  }
}
