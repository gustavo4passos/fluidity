#version 450 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNormal;

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

#define NUM_TOTAL_LIGHTS 8
layout(std140) uniform LightMatrices
{
    LightMatrix lightMatrices[NUM_TOTAL_LIGHTS];
};

uniform sampler2D uCausticPolygon;
uniform sampler2D noSpecularHitMap;

out vec2 fTexCoord;
out vec3 gEyePos;
out flat vec3 noSpecularHitPos;
out flat float gSpecularSurface;

const int lightId = 0;

void main()
{
    fTexCoord = vPos.xy;
    vec4 texValue = textureLod(uCausticPolygon, fTexCoord, 0);
    gEyePos = texValue.xyz;
    noSpecularHitPos = textureLod(noSpecularHitMap, fTexCoord, 0).xyz;
    gSpecularSurface = texValue.a;
    gl_Position = lightMatrices[lightId].prjMatrix * vec4(gEyePos, 1.0);
}
