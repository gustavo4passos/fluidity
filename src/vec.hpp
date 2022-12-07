#pragma once 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct vec2 {
    float x;
    float y;
};

struct vec3 {
    float x;
    float y;
    float z;

    vec3 operator-(const vec3& v)
    {
      return { x - v.x, y - v.y, z - v.z };
    }
    
    operator glm::vec3() const { return { x, y, z }; }
};

struct dVec3 {
    double x;
    double y;
    double z;
};

#pragma pack(push, 1)
struct Vec4 {
  float x;
  float y;
  float z;
  float w;
};

struct Mat4 {
  Vec4 row1;
  Vec4 row2;
  Vec4 row3;
  Vec4 row4;
};

struct CameraData {
  Mat4 viewMatrix;
  Mat4 projectionMatrix;
  Mat4 invViewMatrix;
  Mat4 invProjectionMatrix;
  Mat4 shadowMatrix;
  Vec4 camPosition;
};

struct LightMatrix
{
  Mat4 viewMatrix;
  Mat4 projectionMatrix;
};

struct PointLight {
  Vec4 ambient;
  Vec4 diffuse;
  Vec4 specular;
  Vec4 position;

  static float GetZFar()
  {
    return 100.f;
  }

  static float GetZNear()
  {
    return 1.f;
  }

  glm::mat4 GetProjectionMatrix()
  {
    const float radius = 10.f;
    // return glm::perspective(glm::radians(45.f), 1366.f/768.f, GetZNear(), GetZFar());
    return glm::ortho(-radius, radius, -radius, radius, GetZNear(), GetZFar());
  }

  glm::mat4 GetViewMatrix()
  {
    return glm::lookAt(glm::vec3(position.x, position.y, position.z),
              glm::vec3(0), // directional light, pointing at scene origin
              glm::vec3(0, 1.0, 0)); // Up
  }
};

struct UbMaterial {
  Vec4  ambient;
  Vec4  diffuse;
  Vec4  specular;
  float shininess;
};

struct Material {
  vec3  ambient;
  vec3  diffuse;
  vec3  specular;
  float shininess;
  bool  emissive;
  float reflectiveness;
};

#pragma pack(pop)
