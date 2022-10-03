#pragma once 

struct vec2 {
    float x;
    float y;
};

struct vec3 {
    float x;
    float y;
    float z;
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

struct PointLight {
  Vec4 ambient;
  Vec4 diffuse;
  Vec4 specular;
  Vec4 position;
};

struct LightMatrix
{
  Mat4 viewMatrix;
  Mat4 projectionMatrix;
};

struct UbMaterial {
  Vec4 ambient;
  Vec4 diffuse;
  Vec4 specular;
  float shininess;
};

struct Material {
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
  bool emissive;
  float reflectiveness;
};

#pragma pack(pop)
