#version 450 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNormal;

uniform mat4 projection;
uniform mat4 view;

out vec3 fNormal;

void main() 
{
    gl_Position = projection * view * vec4(vPos, 1.0);
    fNormal = vNormal;  
}
