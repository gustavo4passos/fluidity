#version 450 core

in vec3 fNormal;
out vec3 fragColor;

void main()
{
    // fragColor = vec3(149.0 / 255.0, 69.0 / 255.0, 53.0 / 255.0); // Chestnut color
    fragColor = vec3(fNormal);
}