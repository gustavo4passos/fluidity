#include "camera_controller.hpp"
#include <iostream>

namespace fluidity 
{
  CameraController::CameraController(const Camera& camera)
    : m_camera(camera),
    m_speed(.3f),
    m_movingOnAxis(0.f, 0.f, 0.f),
    m_mouseClicked(false)
  { 
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
        case SDLK_e:
        {
          m_movingOnAxis.y = 1.f;
          break;
        }
        case SDLK_q:
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
        case SDLK_e:
        {
          m_movingOnAxis.y = 0.f;
          break;
        }
        case SDLK_q:
        {
          m_movingOnAxis.y = 0.f;
          break;
        }
        case SDLK_r: // Reset camera position
        {
          m_camera.Reset();
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
    float yaw = m_camera.GetYaw();
    yaw   += mouseMove.x * sensitivity;
    m_camera.SetYaw(yaw);

    SetPitch(m_camera.GetPitch() - mouseMove.y * sensitivity);
  }

  void CameraController::SetPitch(float pitch)
  {
    m_camera.SetPitch(pitch);
    if (pitch >  89.f) m_camera.SetPitch( 89.f);
    if (pitch < -89.f) m_camera.SetPitch(-89.f);
  }

  void CameraController::Update() 
  {
    ProcessMouse();

#if GAME_CONTROLLER_ENABLED
    if (m_gameController.IsControllerConnected())
    {
      AxisStatus axisStatus = m_gameController.GetAxisStatus();

      m_camera.SetYaw(m_camera.GetYaw() + axisStatus.rightAxis.x);
      SetPitch(m_camera.GetPitch() - axisStatus.rightAxis.y);

      m_movingOnAxis.x = axisStatus.leftAxis.x;
      m_movingOnAxis.z = -axisStatus.leftAxis.y;

      m_movingOnAxis.y = -axisStatus.triggers.x;
      if (axisStatus.triggers.y > 0) m_movingOnAxis.y = axisStatus.triggers.y;
    }
#endif

    glm::vec3 direction;
    float yaw = m_camera.GetYaw();
    float pitch = m_camera.GetPitch();
    direction.x = std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch));
    direction.y = std::sin(glm::radians(pitch));
    direction.z = std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch));

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
