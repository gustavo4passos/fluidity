#pragma once
#include "vec.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace fluidity
{
  class Camera
  {
  public:
    Camera(const glm::vec3& position = {}, float fov = 45, float aspectRatio = 16.f / 9.f); 
    Camera(const Camera&) = default;
  
    const glm::mat4 GetProjectionMatrix();
    const glm::mat4 GetViewMatrix();

    glm::vec3 GetPosition() const { return m_position; }
    void SetPosition(const glm::vec3& position) { m_position = position; }

    const glm::vec3& GetFront() const { return m_front; } 
    void SetFront(const glm::vec3& front) { m_front = front; }
    // TODO: This should be configurable

    const float GetFOV() const { return m_fov; };
    void SetFOV(float fov) { m_fov = fov; }
    
    const float GetYaw() const { return m_yaw; }
    void  SetYaw(float yaw) { m_yaw = yaw; }

    const float GetPitch() const { return m_pitch; }
    void SetPitch(float pitch) { m_pitch = pitch; }

    static constexpr float NEAR_PLANE = 0.1f;
    static constexpr float FAR_PLANE  = 1000.f;
    static constexpr glm::vec3 UP = { 0.f, 1.f, 0.f };

  private:
    glm::vec3 m_position;
    glm::vec3 m_front;
    float m_fov;
    float m_aspectRatio;
    float m_yaw;
    float m_pitch;
  };
}
