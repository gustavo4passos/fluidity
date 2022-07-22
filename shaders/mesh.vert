#version 450 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNormal;

#define NUM_TOTAL_LIGHTS            8
struct PointLight
{
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 position;
};

layout(std140) uniform Lights
{
    PointLight lights[NUM_TOTAL_LIGHTS];
    int        u_NumLights;
};

layout(std140) uniform CameraData
{
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 invViewMatrix;
    mat4 invProjectionMatrix;
    mat4 shadowMatrix;
    vec4 camPosition;
};

struct LightMatrix
{
    mat4 viewMatrix;
    mat4 prjMatrix;
};

layout(std140) uniform LightMatrices
{
    LightMatrix lightMatrices[NUM_TOTAL_LIGHTS];
};

uniform mat4 projection;
uniform mat4 view;

out vec3 fFragPos;
out vec4 fFragPosLightSpace;
out vec3 fFragEyePos;
out vec3 fNormal;

void main()
{
    gl_Position = projectionMatrix * viewMatrix * vec4(vPos, 1.0);
    fNormal = vNormal;  
    fFragPos = vPos;
    fFragPosLightSpace = lightMatrices[0].prjMatrix * lightMatrices[0].viewMatrix * vec4(vPos, 1.0);
    fFragEyePos = (viewMatrix * vec4(vPos, 1.0)).xyz;
}
