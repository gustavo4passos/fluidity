#include "camera.hpp"
#include <iostream>

namespace fluidity
{

Camera::Camera(const glm::vec3& position, float fov, float aspectRatio)
  : m_position(position),
  m_front(glm::normalize(glm::vec3(-35.f, -1.f, 0.f) - position)), // TODO: This shouldn't be hardcoded
  m_fov(fov),
  m_aspectRatio(aspectRatio)
{
    // TODO: These should not be hardcoded
    m_yaw = GetDefaultYaw();
    m_pitch = GetDefaultPitch();
} 

 
const glm::mat4 Camera::GetProjectionMatrix()
{
  return glm::perspective(glm::radians(m_fov), m_aspectRatio, NEAR_PLANE, FAR_PLANE);
}

const glm::mat4 Camera::GetViewMatrix()
{
  glm::vec3 up     = glm::vec3(0.f, 1.f, 0.f);
  glm::vec3 target = m_position + m_front;

  glm::mat4 view = glm::lookAt(m_position, target, up);
  return view;
}

void Camera::Reset()
{
  m_position = GetDefaultPosition();
  m_yaw      = GetDefaultYaw();
  m_pitch    = GetDefaultPitch();
}

}
