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

// Shadows
uniform sampler2D uShadowMap;
uniform int uHasShadows;
uniform float uMinShadowBias;
uniform float uMaxShadowBias;
uniform float uShadowIntensity;
uniform int   uUsePcf;

// Material
uniform vec3  uDiffuse;
uniform vec3  uSpecular;
uniform float uShininess;
uniform int   uEmissive;

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
    float shadowBias = max(uMaxShadowBias * (1 - dot(fragNormal, lightDir)), uMinShadowBias);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    vec3 texCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(uShadowMap, texCoords.xy).r;
    float depth = projCoords.z;
    if (depth - shadowBias > closestDepth) return uShadowIntensity;
    return 0.0;
}

float inShadowPCF(vec4 fragPosLightSpace, vec3 fragNormal, vec3 lightDir)
{
    
    float shadowBias = max(uMaxShadowBias * (1 - dot(fragNormal, -lightDir)), uMinShadowBias);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    vec3 texCoords = projCoords * 0.5 + 0.5;
    
    vec2 texelSize = 1.0 / textureSize(uShadowMap, 0);
    float depth = projCoords.z;
    float shadow = 0.0;
    for (int x = -2; x <= 2; x++)
    {
        for (int y = -2; y <= 2; y++)
        {
            float closestDepth = texture(uShadowMap, texCoords.xy + vec2(x, y) * texelSize).r;
            shadow += depth - shadowBias > closestDepth ? 1.0 : 0.0;
        }
    }

    shadow /= 25;
    return shadow * uShadowIntensity;
}

void main()
{
    // vec3 color = vec3(233.0 / 255.0, 116.0 / 255.0, 81.0 / 255.0);
    // float ambient = 0.2;
    // fragColor = vec3(149.0 / 255.0, 69.0 / 255.0, 53.0 / 255.0); // Chestnut color
    vec3 normal   = normalize(fNormal);
    // Point light
    // vec3 lightDir = normalize(lights[0].position.xyz - fFragPos);

    vec3 lightDir = normalize(fFragPos - lights[0].position.xyz);
    vec3 viewDir = normalize(camPosition.xyz - fFragPos);
    vec3 halfwayDir = normalize(viewDir - lightDir);

    float specularStrength = 1;
    vec3 reflectDir = reflect(lightDir, normal);
    vec3 specular = pow(max(dot(viewDir, reflectDir), 0), uShininess) * uSpecular * lights[0].diffuse.xyz;

    float shadow = 1;
    if (uHasShadows == 1)
    {
        if (uUsePcf == 1)
        {
            shadow = 1 - inShadowPCF(fFragPosLightSpace, normal, lightDir);
        }
        else
        {
            shadow = 1 - inShadow(fFragPosLightSpace, normal, lightDir);
        }
    }

    vec3 diffuse = max(dot(normal, -lightDir), 0.0) * lights[0].diffuse.xyz * uDiffuse;
    vec3 ambient = uDiffuse * lights[0].ambient.xyz;
    fragColor = (diffuse + specular) * shadow + ambient;

    // If material is emissive, just show it's diffuse color.
    // Otherwise, use the normal lighting calculations
    // TODO: If material is emissive, the whole lighting calculation can be ignored.
    fragColor = (-1 * uEmissive + 1) * fragColor + uEmissive * uDiffuse;

    vec4 clipSpacePos = projectionMatrix * vec4(fFragEyePos, 1.0);
    fragDepth = vec4(fFragEyePos, 1.0).z;
    gl_FragDepth = clipSpacePos.z / clipSpacePos.w;
}