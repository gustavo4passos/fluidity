#version 450

in  vec3 vPos;
out vec4 fFragPos;

#define NUM_TOTAL_LIGHTS 8
struct LightMatrix
{
    mat4 viewMatrix;
    mat4 prjMatrix;
};

layout(std140) uniform LightMatrices
{
    LightMatrix lightMatrices[NUM_TOTAL_LIGHTS];
};

uniform mat4 uLightMatrix;
uniform mat4 model;

void main()
{
    fFragPos = lightMatrices[0].prjMatrix * lightMatrices[0].viewMatrix * model * vec4(vPos, 1.0);
    gl_Position = fFragPos;
}