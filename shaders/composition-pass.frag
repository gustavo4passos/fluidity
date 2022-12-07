//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//                                .--,       .--,
//                               ( (  \.---./  ) )
//                                '.__/o   o\__.'
//                                   {=  ^  =}
//                                    >  -  <
//     ___________________________.""`-------`"".____________________________
//    /                                                                      \
//    \    This file is part of Banana - a graphics programming framework    /
//    /                    Created: 2018 by Nghia Truong                     \
//    \                      <nghiatruong.vn@gmail.com>                      /
//    /                      https://ttnghia.github.io                       \
//    \                        All rights reserved.                          /
//    /                                                                      \
//    \______________________________________________________________________/
//                                  ___)( )(___
//                                 (((__) (__)))
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// fragment shader, compositing pass
#version 410 core

#define NUM_TOTAL_LIGHTS 8
#define SHADOW_BIAS      -0.01
#define DEPTH_BIAS       0.0001

#define FIXED_SPOT

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

struct LightMatrix
{
    mat4 viewMatrix;
    mat4 prjMatrix;
};

layout(std140) uniform LightMatrices
{
    LightMatrix lightMatrices[NUM_TOTAL_LIGHTS];
};

layout(std140) uniform Material
{
    vec4  ambient;
    vec4  diffuse;
    vec4  specular;
    float shininess;
} material;

layout(std140) uniform CameraData
{
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 invViewMatrix;
    mat4 invProjectionMatrix;
    mat4 shadowMatrix;
    vec4 camPosition;
};

uniform int       u_HasSolid;
uniform sampler2D u_SolidDepthMap;

uniform int u_ScreenWidth;
uniform int u_ScreenHeight;

uniform samplerCube u_SkyBoxTex;
uniform sampler2D   u_DepthTex;
uniform sampler2D   u_ThicknessTex;
uniform sampler2D   u_FrontNormalTex;
uniform sampler2D   u_BackNormalTex;
uniform sampler2D   u_BackgroundTex;

uniform int       u_HasShadow;
uniform float     u_ShadowIntensity;
uniform int       u_VisualizeShadowRegion;
uniform sampler2D u_SolidShadowMaps[NUM_TOTAL_LIGHTS];
uniform sampler2D u_FluidShadowMaps[NUM_TOTAL_LIGHTS];
uniform sampler2D u_FluidShadowThickness[NUM_TOTAL_LIGHTS];

uniform float u_ReflectionConstant;
uniform float u_AttennuationConstant;
uniform int   u_TransparentFluid;

// fluidity
uniform float uMinShadowBias;
uniform float uMaxShadowBias;
uniform float uRefractionModifier;
uniform int   uUseRefractionMask;
uniform int   uTwoSidedRefractions; 
uniform float uRefractiveIndex;
uniform float u_ParticleRadius;
uniform int   uBackSurfaceSpecular;

// in/out
in vec2  f_TexCoord;
out vec4 fragColor;
out float failedToFind;

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// const variables
const float farZ  = 1000.0f;
const float nearZ = 0.1f;

const float refractiveIndex = 1.33;
const float eta             = 1.0 / refractiveIndex; // Ratio of indices of refraction
const float fresnelPower    = 5.0;
const float F               = ((1.0 - eta) * (1.0 - eta)) / ((1.0 + eta) * (1.0 + eta));

#define RENDER_BLUE

#ifdef RENDER_BLUE
const float k_r = 0.5f;
const float k_g = 0.2f;
const float k_b = 0.05f;
#else
const float k_r = 0.8f;
const float k_g = 0.2f;
const float k_b = 0.9f;
#endif
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
vec3 uvToEye(vec2 texCoord, float eyeDepth)
{
    float x  = texCoord.x * 2.0 - 1.0;
    float y  = texCoord.y * 2.0 - 1.0;
    float zn = ((farZ + nearZ) / (farZ - nearZ) * eyeDepth + 2 * farZ * nearZ / (farZ - nearZ)) / eyeDepth;

    vec4 clipPos = vec4(x, y, zn, 1.0f);
    vec4 viewPos = invProjectionMatrix * clipPos;
    return viewPos.xyz / viewPos.w;
}

vec4 uvToSpace(vec2 texCoord, float eyeDepth)
{
    float x  = texCoord.x * 2.0 - 1.0;
    float y  = texCoord.y * 2.0 - 1.0;
    float zn = ((farZ + nearZ) / (farZ - nearZ) * eyeDepth + 2 * farZ * nearZ / (farZ - nearZ)) / eyeDepth;

    vec4 clipPos = vec4(x, y, zn, 1.0f);
    vec4 viewPos = invProjectionMatrix * clipPos;
    vec4 eyePos  = viewPos.xyzw / viewPos.w;

    vec4 worldPos = invViewMatrix * eyePos;
    return worldPos;
}

vec3 computeAttennuation(float thickness)
{
    return vec3(exp(-k_r * thickness), exp(-k_g * thickness), exp(-k_b * thickness));
}

vec3 eyeToScreen(vec3 v)
{
  vec4 projectedV = projectionMatrix * vec4(v, 1.0);
  projectedV.xyz /= projectedV.w;
  projectedV.xyz *= 0.5;
  projectedV.xyz += 0.5;
  return projectedV.xyz;
}

float deltaZ(vec3 Pf, vec3 Tf, float t)
{
  vec3 Pt = Pf + t * Tf;
  vec3 projectedPt = eyeToScreen(Pt);
  return Pt.z - texture(u_DepthTex, projectedPt.xy).g; 
}

bool isValidBackfacePixel(vec2 texCoord)
{
  vec3 value = texture(u_DepthTex, texCoord).rgb;
  return !(value.b < 0);
}

vec3 findRefractionIntersection(vec2 texCoord, float depth, vec3 viewDir)
{
  float t0 = 0;
  vec3 frontNormal = texture(u_FrontNormalTex, texCoord).xyz;
  vec3 Tf = normalize(refract(-viewDir, frontNormal, uRefractiveIndex));
  vec3 Pf = uvToEye(f_TexCoord, depth);

  float ts = t0;
  float te = (-farZ - Pf.z) / Tf.z;

  while(!isValidBackfacePixel(eyeToScreen(Pf + te * Tf).xy))
  {
    te *= 0.5;
  }

  const float epsilon = 0.1;
  float intersection = 0;

  int i = 0;
  // Find first valid tex

  while(true)
  {
    float deltaZTs = deltaZ(Pf, Tf, ts);
    float deltaZTe = deltaZ(Pf, Tf, te);
    float tStar = ts - ((te - ts) / (deltaZTe - deltaZTs)) * deltaZTs;

    if (abs(deltaZ(Pf, Tf, tStar)) < epsilon)
    {
      failedToFind = 0;
      intersection = tStar;
      break;
    }

    if (tStar < te) ts = tStar;
    else te = tStar;

    i++;
    // TODO: Should this always converge?
    // Give up after 10 iterations
    if (i == 10)
    {
        failedToFind = 1;
        intersection = tStar;
        break;
    }
  }

  return Pf + intersection * Tf;
}


// Fluidity - Ray-plane intersections
// Find intersection between a ray and planes in the world.
// Used to find where a refraction ray hits.
uniform int uNPlanes;
const int MAX_N_PLANES = 5;

struct PlaneData 
{
  vec3 origin;
  vec3 normal;
  vec3 v0;
  vec3 v1;
  vec3 v2;
  vec3 v3;
};

uniform PlaneData uPlanes[MAX_N_PLANES];

struct IntersectionData
{
  bool intersects;
  float t;
};

// Finds t such that t * lineV is the projected vector
// Line V that definse the line span(v)
float ProjectVectorOntoLineT(vec3 lineV, vec3 v)
{
  return dot(lineV, v) / dot(lineV, lineV); 
}

IntersectionData FindRayPlaneIntersection(PlaneData plane, vec3 rayOrigin, vec3 rayDirection)
{
  IntersectionData result;
  result.intersects = false;
  result.t = 0;

  float denom = dot(plane.normal, rayDirection);
  if (abs(denom) < 1e-6) return result;

  vec3 rayOriginToPlaneCenter = plane.origin - rayOrigin;
  result.t = dot(rayOriginToPlaneCenter, plane.normal) / denom;

  // Moves vertices to origin to find projection
  vec3 v1o = plane.v1 - plane.v0;
  vec3 v3o = plane.v3 - plane.v0;
  vec3 into = (rayOrigin + rayDirection * result.t) - plane.v0;

  float tx = ProjectVectorOntoLineT(v1o, into);
  float ty = ProjectVectorOntoLineT(v3o, into);

  // Because of the particle radius being customizable, the ray might need to go "backwards".
  // This will only be acceptable if the distance the ray has to travel is less than 
  // particle radius * 2
  if (result.t < 0 && result.t < -u_ParticleRadius * 2) return result;

  // If the projection of the intersection vector falls outside on any of the projection, 
  // the point is not inside the plane section
  if (tx >= 0 && tx <= 1 && ty >= 0 && ty <= 1)
  {
    result.intersects = true;
  }

  return result;
}

const int lightID = 0;
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void main()
{
    float eyeDepth  = texture(u_DepthTex, f_TexCoord).r;
    vec3  backColor = texture(u_BackgroundTex, f_TexCoord).xyz;

    if(u_HasSolid == 1) {
#ifdef FIXED_SPOT
        //        float dx            = 1.0f / 1080.0f;
        //        float dy            = 1.0f / 1920.0f;
        float dx            = 1.0f / u_ScreenWidth;
        float dy            = 1.0f / u_ScreenHeight;
        float solidDepth_nx = texture(u_SolidDepthMap, f_TexCoord + vec2(-dx,  0)).r;
        float solidDepth_px = texture(u_SolidDepthMap, f_TexCoord + vec2( dx,  0)).r;
        float solidDepth_ny = texture(u_SolidDepthMap, f_TexCoord + vec2(  0, -dy)).r;
        float solidDepth_py = texture(u_SolidDepthMap, f_TexCoord + vec2(  0,  dy)).r;
#endif
        float solidDepth = texture(u_SolidDepthMap, f_TexCoord).r;
        if(eyeDepth < solidDepth + DEPTH_BIAS
#ifdef FIXED_SPOT
           && eyeDepth < solidDepth_nx + DEPTH_BIAS
           && eyeDepth < solidDepth_px + DEPTH_BIAS
           && eyeDepth < solidDepth_ny + DEPTH_BIAS
           && eyeDepth < solidDepth_py + DEPTH_BIAS
#endif
           ) {
                fragColor = vec4(backColor, 1.0f);
                return;
        }
    }

    vec3  N         = texture(u_FrontNormalTex, f_TexCoord).xyz;
    float thickness = texture(u_ThicknessTex, f_TexCoord).r;

    vec3 position = uvToEye(f_TexCoord, eyeDepth);
    vec3 viewer   = normalize(-position.xyz);

    vec3  lightDir = normalize(vec3(viewMatrix * lights[lightID].position) - position);
    vec3  H        = normalize(lightDir + viewer);
    float specular = pow(max(0.0f, dot(H, N)), material.shininess);
    float diffuse  = max(0.0f, dot(lightDir, N)) * 1.0f;

    //Background Only Pixels
    if(eyeDepth > 0.0f || eyeDepth < -1000.0f) {
        fragColor = vec4(backColor, 1.0f);
        return;
    }

    vec3 shadowColor = vec3(1.0);
#if ENABLE_COMPOSITION_SHADOWS
    if(u_HasShadow == 1) {
        vec4  worldCoord        = uvToSpace(f_TexCoord, eyeDepth);
        vec4  lightCoord        = lightMatrices[lightID].viewMatrix * worldCoord;
        vec4  fragPosLightSpace = lightMatrices[lightID].prjMatrix * lightCoord;
        float inSolidShadow     = 0.0;

        vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
        float depth = projCoords.z;
        projCoords = projCoords * 0.5 + 0.5; // Transform to [0,1] range
        vec2 shadowTexCoord = clamp(projCoords.xy, 0.0, 1.0);
        vec2 texelSize      = 1.0 / textureSize(u_SolidShadowMaps[lightID], 0);
        for(int x = -1; x <= 1; ++x) {
            for(int y = -1; y <= 1; ++y) {
                float shadowBias = max(uMaxShadowBias * (1 - dot(N, lightDir)), uMinShadowBias);
                float closestDepth = texture(u_SolidShadowMaps[lightID], shadowTexCoord + vec2(x, y) * texelSize).r;
                inSolidShadow += depth - shadowBias > closestDepth ? 1.0 : 0.0;
            }
        }
        inSolidShadow /= 9.0;
        inSolidShadow *= u_ShadowIntensity;

        float sampleFluidDepth     = texture(u_FluidShadowMaps[lightID], shadowTexCoord).r;
        float inFluidShadow        = (lightCoord.z < sampleFluidDepth + SHADOW_BIAS) ? 1.0 : 0.0;
        float fluidShadowThickness = texture(u_FluidShadowThickness[lightID], shadowTexCoord).r;

        inFluidShadow = 0; // TODO: Remove me
        vec3 fluidShadowColor = inFluidShadow > 0 ? computeAttennuation(u_ShadowIntensity * fluidShadowThickness * 0.2f) : vec3(1.0);
        shadowColor = (1.0 - u_ShadowIntensity * inSolidShadow) * fluidShadowColor;

        if((u_VisualizeShadowRegion == 1) && (projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)) {
            shadowColor *= vec3(0.1, 0.1, 1.0);
        }
    }
#endif

    if(u_TransparentFluid == 0) {
        fragColor = vec4(vec3(material.ambient + material.diffuse * diffuse + vec4(0, 0, 0.2, 0) + material.specular * specular) * shadowColor, 1.0);
        return;
    }

    //Fresnel Reflection
    float fresnelRatio    = clamp(F + (1.0 - F) * pow((1.0 - dot(viewer, N)), fresnelPower), 0, 1);
    // Fluidity - Shouldn't reflectionDir be in world space? This is what
    // I'm doing here.
    // TODO: For some reason world-space reflection is failing from some angles.
    // I'm switching to eye-space for now until I find out why.
    vec3 viewDirWorld = normalize(uvToSpace(f_TexCoord, eyeDepth).xyz - (camPosition).xyz);
    vec3 frontNormal = texture(u_FrontNormalTex, f_TexCoord).xyz;
    vec3 frontNormalWorld = normalize(mat3(transpose(viewMatrix)) * frontNormal);
#if ENABLE_WORLD_SPACE_REFLECTION
    // vec3 reflectionDir = reflect(viewDirWorld, frontNormalWorld);
#endif
    vec3 reflectionDir = reflect(-viewer, N);
    reflectionDir = normalize(reflectionDir);
    vec3 reflectionColor = texture(u_SkyBoxTex, reflectionDir).xyz;

    //Color Attenuation from Thickness (Beer's Law)
    vec3 colorAttennuation = computeAttennuation(thickness * 5.0f);
    colorAttennuation = mix(vec3(1, 1, 1), colorAttennuation, u_AttennuationConstant);
    vec3 refractionDir = normalize(refract(-viewer, N, uRefractiveIndex));
    vec2 refractionDisplacement = refractionDir.xy * uRefractionModifier * thickness * u_AttennuationConstant * 0.1f;
    vec3 refractionColor;

    // Fluidity - Two sided refractions
    if (uTwoSidedRefractions == 1)
    {
      vec3 intersection = findRefractionIntersection(f_TexCoord, eyeDepth, viewer);
      vec3 projectedIntersection = eyeToScreen(intersection);

      vec3 Tf                   = normalize(refract(-viewer, frontNormal, eta));
      vec3 normalAtIntersection = texture(u_BackNormalTex, projectedIntersection.xy).xyz;
      vec3 finalRefractionDir   = normalize(refract(Tf, normalAtIntersection, eta));

      vec3 TfWorld = normalize(refract(viewDirWorld, frontNormalWorld, uRefractiveIndex));
      vec3 normalAtIntersectionWorld = normalize(mat3(transpose(viewMatrix)) * normalAtIntersection);
      vec3 refractionDirWorld = normalize(refract(TfWorld, normalAtIntersectionWorld, uRefractiveIndex));
      refractionColor         = texture(u_SkyBoxTex, refractionDirWorld).xyz * colorAttennuation;

      if (uBackSurfaceSpecular == 1)
      {
        // Specular at the back surface
        vec3  viewerBack = normalize(-intersection);
        vec3  H        = normalize(lightDir + viewerBack);
        specular += pow(max(0.0f, dot(H, normalAtIntersection)), material.shininess);
      }

      float closestIntersectionT = -farZ;
      for(int i = 0; i < uNPlanes; i++)
      {
        IntersectionData iData = FindRayPlaneIntersection(uPlanes[i], intersection, finalRefractionDir);
        if (iData.intersects)
        {
          vec3 pIntersection = intersection + finalRefractionDir * iData.t;
          vec3 pIntersectionScreen = eyeToScreen(pIntersection);
          float solidDepthAtRefractionPoint = texture(u_SolidDepthMap, pIntersectionScreen.xy).r;
          if (pIntersectionScreen.z > closestIntersectionT)
          {
            refractionColor = texture(u_BackgroundTex, pIntersectionScreen.xy).xyz * colorAttennuation;
            closestIntersectionT = pIntersection.z;
          }
        }
      }
    }
    else
    {
      // Fluidity - Refraction mask
      if (uUseRefractionMask == 1)
      {
          bool validDisplacement = false;
          for (int i = 0; i < 5; i++)
          {
              float fluidDepthAtRefractionPoint = texture(u_DepthTex, f_TexCoord + refractionDisplacement).r;
              float solidDepthAtRefractionPoint = texture(u_SolidDepthMap, f_TexCoord + refractionDisplacement).r;
              if ((fluidDepthAtRefractionPoint > 0 || fluidDepthAtRefractionPoint < -1000.f) ||
                  solidDepthAtRefractionPoint >= fluidDepthAtRefractionPoint)
              {
                  refractionDisplacement *= 0.5;
              }
              else
              {
                  validDisplacement = true;
                  break;
              }

              if (!validDisplacement) refractionDisplacement = vec2(0);
          }
      }

      refractionColor = colorAttennuation * texture(u_BackgroundTex, f_TexCoord + refractionDisplacement).xyz;
    }

    fresnelRatio = mix(fresnelRatio, 1.0, u_ReflectionConstant);
    vec3 finalColor = (mix(refractionColor, reflectionColor, fresnelRatio) + specular * material.specular.xyz) * shadowColor;
    
    fragColor = vec4(finalColor, 1);
}
