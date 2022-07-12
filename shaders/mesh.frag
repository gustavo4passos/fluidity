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
in vec4 fFragPosLightSpace;

out vec3 fragColor;
out float fragDepth;

uniform sampler2D uShadowMap;
uniform mat4 uLightMatrix;
uniform int uHasShadows;

float calculateAttenuation(vec3 fragPos, vec3 lightPos)
{
    float distance = length(lightPos - fragPos);
    float quadratic = 0.00032; //0.032;
    float constant  = 1.0;
    float linear    = 0.001;

    float attenuation = 1.0 / (constant + linear * distance + quadratic * distance * distance);
    return attenuation;
}

float inShadow(vec4 fragPosLightSpace, vec3 fragNormal, vec3 lightDir)
{
    float shadowBiasP = 0.007;
    float shadowBias = max(0.008 * (1 - dot(fragNormal, lightDir)), 0.0005);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    vec3 texCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(uShadowMap, texCoords.xy).r;
    float depth = projCoords.z;
    if (depth - shadowBias > closestDepth) return 0.7;
    return 0.0;
}

float inShadowPCF(vec4 fragPosLightSpace, vec3 fragNormal, vec3 lightDir)
{
    
    float shadowBiasP = 0.007;
    float shadowBias = max(0.01 * (1 - dot(fragNormal, lightDir)), 0.0005);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    vec3 texCoords = projCoords * 0.5 + 0.5;
    
    vec2 texelSize = 1.0 / textureSize(uShadowMap, 0);
    float depth = projCoords.z;
    float shadow = 0.0;
    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            float closestDepth = texture(uShadowMap, texCoords.xy + vec2(x, y) * texelSize).r;
            shadow += depth - shadowBias > closestDepth ? 1.0 : 0.0;
        }
    }

    shadow /= 9.0;
    return shadow;
}

void main()
{
    vec3 color = vec3(233.0 / 255.0, 116.0 / 255.0, 81.0 / 255.0);
    float ambient = 0.2;
    // fragColor = vec3(149.0 / 255.0, 69.0 / 255.0, 53.0 / 255.0); // Chestnut color
    vec3 normal   = normalize(fNormal);
    // Point light
    // vec3 lightDir = normalize(lights[0].position.xyz - fFragPos);
    
    // Directional light
    vec3 lightDir = normalize(lights[0].position.xyz);
    float shadow = 1;
    if (uHasShadows == 1)
    {
        shadow = 1 - inShadowPCF(fFragPosLightSpace, normal, lightDir);
    }
    fragColor     = (max(dot(normal, lightDir), 0.0) * shadow + ambient) * color;

    vec4 clipSpacePos = projectionMatrix * vec4(fFragEyePos, 1.0);
    fragDepth = -(projectionMatrix * vec4(fFragEyePos, 1.0)).z;
    gl_FragDepth = clipSpacePos.z / clipSpacePos.w;
}