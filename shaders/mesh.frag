#version 450 core

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

in vec3 fNormal;
in vec3 fFragPos;
in vec3 fFragEyePos;

out vec3 fragColor;
out float fragDepth;

void main()
{
    // fragColor = vec3(149.0 / 255.0, 69.0 / 255.0, 53.0 / 255.0); // Chestnut color
    vec3 normal   = normalize(fNormal);
    vec3 lightDir = normalize(lights[0].position.xyz - fFragPos);
    fragColor     = max(dot(normal, lightDir), 0.0) * vec3(233.0 / 255.0, 116.0 / 255.0, 81.0 / 255.0) +
        vec3(0.1, 0.1, 0.1);

    vec4 clipSpacePos = projectionMatrix * vec4(fFragEyePos, 1.0);
    fragDepth = -(projectionMatrix * vec4(fFragEyePos, 1.0)).z;
    gl_FragDepth = clipSpacePos.z / clipSpacePos.w;
}