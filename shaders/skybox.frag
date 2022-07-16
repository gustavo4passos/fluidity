#version 450

in vec3 fTexCoords;

out vec3 fFragColor;
uniform samplerCube skybox;

void main()
{
    fFragColor = texture(skybox, fTexCoords).xyz;
}