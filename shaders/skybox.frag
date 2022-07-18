#version 450

in vec3 fTexCoords;

out vec3 fFragColor;
out float fragDepth;

uniform samplerCube skybox;

void main()
{
    fFragColor = texture(skybox, fTexCoords).xyz;
    fragDepth = -10000000; // TODO: This should not be hardcoded
}