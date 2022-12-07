#version 450 core

#define NUM_TOTAL_LIGHTS 8

uniform sampler2D uFluidDepth;
uniform sampler2D uFrontFaceNormals;
uniform sampler2D uBackFaceNormals;
uniform sampler2D uSolidDepth;

uniform float uNearZ;
uniform float uFarZ;

uniform float uRefractiveIndex;

in vec2 f_TexCoord;
// The W component is used to represent specularities
out vec4 photonFinalLocation;
out vec3 noSpecularHit;
out float failedToFind;

const int lightId = 0;

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

// vec3 uvToEye(vec2 texCoord, float eyeDepth, int id)
// {
//     float x  = texCoord.x * 2.0 - 1.0;
//     float y  = texCoord.y * 2.0 - 1.0;
//     float zn = ((uFarZ + uNearZ) / (uFarZ - uNearZ) * eyeDepth + 2 * uFarZ * uNearZ / (uFarZ - uNearZ)) / eyeDepth;

//     vec4 clipPos = vec4(x, y, zn, 1.0f);
//     vec4 viewPos = inverse(lightMatrices[id].prjMatrix) * clipPos;
//     return viewPos.xyz / viewPos.w;
// }

vec3 uvToEyeOrtho(vec2 texCoord, float eyeDepth, int id)
{
    const float uLeft   = -10.f;
    const float uRight  =  10.f;
    const float uTop    =  10.f;
    const float uBottom = -10.f;

    float x  = texCoord.x * 2.0 - 1.0;
    float y  = texCoord.y * 2.0 - 1.0;

    x += ((uRight + uLeft) / (uRight - uLeft));
    x /= (2 / (uRight - uLeft));

    y += ((uTop + uBottom) / (uTop - uBottom));
    y /= (2 / (uTop - uBottom));

    return vec3(x, y, eyeDepth);
}

vec4 uvToWorldOrtho(vec2 texCoord, float eyeDepth, int id)
{
  vec4 eyePos = vec4(uvToEyeOrtho(texCoord, eyeDepth, id), 1.0);
  return inverse(lightMatrices[id].viewMatrix) * eyePos;
}

// vec4 uvToWorld(vec2 texCoord, float eyeDepth, int id)
// {
//     float x  = texCoord.x * 2.0 - 1.0;
//     float y  = texCoord.y * 2.0 - 1.0;
//     float zn = ((uFarZ + uNearZ) / (uFarZ - uNearZ) * eyeDepth + 2 * uFarZ * uNearZ / (uFarZ - uNearZ)) / eyeDepth;

//     vec4 clipPos = vec4(x, y, zn, 1.0f);
//     vec4 viewPos = inverse(lightMatrices[id].prjMatrix) * clipPos;
//     vec4 eyePos  = viewPos.xyzw / viewPos.w;

//     vec4 worldPos = inverse(lightMatrices[id].viewMatrix) * eyePos;
//     return worldPos;
// }

vec3 eyeToScreen(vec3 v, int id)
{
  vec4 projectedV = lightMatrices[id].prjMatrix * vec4(v, 1.0);
  projectedV.xyz /= projectedV.w;
  projectedV.xyz *= 0.5;
  projectedV.xyz += 0.5;
  return projectedV.xyz;
}

float deltaZ(vec3 Pf, vec3 Tf, float t, int id)
{
  vec3 Pt = Pf + t * Tf;
  vec3 projectedPt = eyeToScreen(Pt, id);
  return Pt.z - texture(uFluidDepth, projectedPt.xy).g; 
}

bool isValidBackfacePixel(vec2 texCoord)
{
  vec3 value = texture(uFluidDepth, texCoord).rgb;
  return !(value.b < 0);
}

vec3 findRefractionIntersection(vec2 texCoord, float depth, vec3 viewDir, int id)
{
  float t0 = 0;
  vec3 frontNormal = texture(uFrontFaceNormals, texCoord).xyz;
  vec3 Tf = normalize(refract(-viewDir, frontNormal, uRefractiveIndex));
  vec3 Pf = uvToEyeOrtho(texCoord, depth, id);

  float ts = t0;
  float te = (-uFarZ - Pf.z) / Tf.z;

  while(!isValidBackfacePixel(eyeToScreen(Pf + te * Tf, id).xy))
  {
    te *= 0.5;
  }

  const float epsilon = 0.1;
  float intersection = 0;

  int i = 0;
  // Find first valid tex

  while(true)
  {
    float deltaZTs = deltaZ(Pf, Tf, ts, id);
    float deltaZTe = deltaZ(Pf, Tf, te, id);
    float tStar = ts - ((te - ts) / (deltaZTe - deltaZTs)) * deltaZTs;

    if (abs(deltaZ(Pf, Tf, tStar, id)) < epsilon)
    {
      intersection = tStar;
      failedToFind = 0;
      break;
    }

    if (tStar < te) ts = tStar;
    else te = tStar;

    i++;
    // TODO: Should this always converge?
    // Give up after 10 iterations
    if (i == 30)
    {
        intersection = tStar;
        failedToFind = 1;
        break;
    }
  }

  return Pf + intersection * Tf;
}

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
  // Ray and plane are paralel
  // denom should be zero, but due to float point imprecisions and error accumulation,
  // a very small number is used (1e-6))
  if (abs(denom) < 1e-6) return result;

  vec3 rayOriginToPlaneCenter = plane.origin - rayOrigin;
  result.t = dot(rayOriginToPlaneCenter, plane.normal) / denom;

  // Moves vertices to origin to find projection
  vec3 v1o = plane.v1 - plane.v0;
  vec3 v3o = plane.v3 - plane.v0;
  vec3 into = (rayOrigin + rayDirection * result.t) - plane.v0;

  float tx = ProjectVectorOntoLineT(v1o, into);
  float ty = ProjectVectorOntoLineT(v3o, into);

  // If the projection of the intersection vector falls outside on any of the projection, 
  // the point is not inside the plane section
  if (tx >= 0 && tx <= 1 && ty >= 0 && ty <= 1)
  {
    result.intersects = true;
  }

  return result;
}

void main()
{
  float solidDepth      = texture(uSolidDepth, f_TexCoord).r;
  float fluidFrontDepth = texture(uFluidDepth, f_TexCoord).r;

  // Fragment is a refractive surface
  if (fluidFrontDepth >= solidDepth) 
  {
    vec3 frontNormal           = texture(uFrontFaceNormals, f_TexCoord).xyz;

    // Ignore extremities
    float zNormalThreshold = 1;
    if (frontNormal.z < zNormalThreshold)
    {
      vec3 fragPosEye            = uvToEyeOrtho(f_TexCoord, fluidFrontDepth, lightId);
      noSpecularHit = fragPosEye;
      vec3 viewDir               = normalize(-fragPosEye);
      vec3 intersection          = findRefractionIntersection(f_TexCoord, fluidFrontDepth, viewDir, lightId);
      vec3 projectedIntersection = eyeToScreen(intersection, lightId);
      vec3 normalAtIntersection  = texture(uBackFaceNormals, projectedIntersection.xy).xyz;
      vec3 Tf                    = normalize(refract(-viewDir, frontNormal, uRefractiveIndex));
      vec3 finalRefractionDir    = normalize(refract(Tf, normalAtIntersection, uRefractiveIndex));
      
      vec3 fragPosWorld = uvToWorldOrtho(f_TexCoord, fluidFrontDepth, lightId).xyz;
      vec3 viewDirWorld = normalize(fragPosWorld - lights[lightId].position.xyz);
      vec3 frontNormalWorld = normalize(mat3(transpose(inverse(lightMatrices[lightId].viewMatrix))) * frontNormal);

      vec3 normalAtIntersectionWorld = normalize(mat3(transpose(inverse(lightMatrices[lightId].viewMatrix))) * normalAtIntersection);
      vec3 TfWorld               = normalize(refract(viewDirWorld, frontNormalWorld, uRefractiveIndex));
      vec3 refractionDirWorld    = normalize(refract(TfWorld, normalAtIntersectionWorld, uRefractiveIndex));

      // TODO: How to deal when not using planes, or parametrically defined shapes?
      // float depthAtIntersection = texture(uSolidDepth, projectedIntersection.xy).r;
      // photonFinalLocation  = vec4(uvToEyeOrtho(projectedIntersection.xy, depthAtIntersection, lightId).xyz, 1.0);
      photonFinalLocation  = vec4(-uFarZ * 2, -uFarZ * 2, -uFarZ * 2, -1.0);

      float closestIntersectionT = -uFarZ;
      for (int i = 0; i < uNPlanes; i++)
      {
        IntersectionData iData = FindRayPlaneIntersection(uPlanes[i], intersection, finalRefractionDir);

        if (iData.intersects)
        {
          vec3 pIntersection = intersection + refractionDirWorld * iData.t;
          vec3 pIntersectionScreen = eyeToScreen(pIntersection, lightId);
          float solidDepthAtRefractionPoint = texture(uSolidDepth, pIntersectionScreen.xy).r;
          if (pIntersectionScreen.z > closestIntersectionT)
          {
            float depthAtPIntersection = texture(uSolidDepth, pIntersectionScreen.xy).r;
            photonFinalLocation = vec4(uvToEyeOrtho(pIntersectionScreen.xy, depthAtPIntersection, lightId), 1.0);
            closestIntersectionT = pIntersection.z;
          }
        }
      }
      return;
    }
  }
  
  if (solidDepth > -uFarZ)
  {
    photonFinalLocation = vec4(uvToEyeOrtho(f_TexCoord, solidDepth, lightId).xyz, 0.0);
    noSpecularHit = photonFinalLocation.xyz;
  }
  else 
  {
    photonFinalLocation = vec4(-uFarZ * 2, -uFarZ * 2, -uFarZ * 2, -1.0);
    noSpecularHit = photonFinalLocation.xyz;
  }
}
