#pragma once
#include "vec.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace fluidity
{
  class Camera
  {
  public:
    Camera(const glm::vec3& position, float fov = 15, float aspectRatio = 16.f / 9.f); 
  
    const glm::mat4 GetProjectionMatrix();
    const glm::mat4 GetViewMatrix();

    const glm::vec3& GetPosition() { return m_position; }
    void SetPosition(const glm::vec3& position) { m_position = position; }

    const glm::vec3& GetFront() { return m_front; } 
    void SetFront(const glm::vec3& front) { m_front = front; }
    // TODO: This should be configurable

    const float GetFOV() { return m_fov; };
    void SetFOV(float fov) { m_fov = fov; }
    
    static constexpr float NEAR_PLANE = 0.1f;
    static constexpr float FAR_PLANE  = 1000.f;
    static constexpr glm::vec3 UP = { 0.f, 1.f, 0.f };

  private:
    glm::vec3 m_position;
    glm::vec3 m_front;
    float m_fov;
    float m_aspectRatio;
  };
}
