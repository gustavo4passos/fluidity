#version 450

in  vec3 vPos;
out vec4 fFragPos;

uniform mat4 uLightMatrix;

void main()
{
    fFragPos = uLightMatrix * vec4(vPos, 1.0);
    gl_Position = fFragPos;
}