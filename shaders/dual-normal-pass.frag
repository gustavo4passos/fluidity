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
// fragment shader, normal pass
#version 410 core

layout(std140) uniform CameraData
{
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 invViewMatrix;
    mat4 invProjectionMatrix;
    mat4 shadowMatrix;
    vec4 camPosition;
};

uniform sampler2D u_DepthTex;
uniform int       u_ScreenWidth;
uniform int       u_ScreenHeight;
uniform int       u_frontAndBackNormals;

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
const float far  = 1000.0f;
const float near = 0.1f;

in vec2  f_TexCoord;
out vec3 outNormalFront;
out vec3 outNormalBack;

vec3 uvToEye(vec2 texCoord, float depth)
{
    float x  = texCoord.x * 2.0 - 1.0;
    float y  = texCoord.y * 2.0 - 1.0;
    float zn = ((far + near) / (far - near) * depth + 2 * far * near / (far - near)) / depth;

    vec4 clipPos = vec4(x, y, zn, 1.0f);
    vec4 viewPos = inverse(projectionMatrix) * clipPos;
    return viewPos.xyz / viewPos.w;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void main()
{
    float pixelWidth  = 1 / float(u_ScreenWidth);
    float pixelHeight = 1 / float(u_ScreenHeight);
    float x           = f_TexCoord.x;
    float xp          = f_TexCoord.x + pixelWidth;
    float xn          = f_TexCoord.x - pixelWidth;
    float y           = f_TexCoord.y;
    float yp          = f_TexCoord.y + pixelHeight;
    float yn          = f_TexCoord.y - pixelHeight;

    vec2 depth   = texture(u_DepthTex, vec2(x, y)).rg;
    vec2 depthxp = texture(u_DepthTex, vec2(xp, y)).rg;
    vec2 depthxn = texture(u_DepthTex, vec2(xn, y)).rg;
    vec2 depthyp = texture(u_DepthTex, vec2(x, yp)).rg;
    vec2 depthyn = texture(u_DepthTex, vec2(x, yn)).rg;

    if(depth.r < -999.0f) {
        outNormalFront = vec3(0, 1, 0);
        outNormalBack  = vec3(0, 1, 0);
        return;
    }

    // Calc front normal
    {
      vec3 position   = uvToEye(vec2(x, y), depth.r);
      vec3 positionxp = uvToEye(vec2(xp, y), depthxp.r);
      vec3 positionxn = uvToEye(vec2(xn, y), depthxn.r);
      vec3 dxl        = position - positionxn;
      vec3 dxr        = positionxp - position;

      vec3 dx = (abs(dxr.z) < abs(dxl.z)) ? dxr : dxl;

      vec3 positionyp = uvToEye(vec2(x, yp), depthyp.r);
      vec3 positionyn = uvToEye(vec2(x, yn), depthyn.r);
      vec3 dyb        = position - positionyn;
      vec3 dyt        = positionyp - position;

      vec3 dy = (abs(dyt.z) < abs(dyb.z)) ? dyt : dyb;

      //Compute Gradients of Depth and Cross Product Them to Get Normal
      vec3 N = normalize(cross(dx, dy));
      if(isnan(N.x) || isnan(N.y) || isnan(N.y) ||
         isinf(N.x) || isinf(N.y) || isinf(N.z)) {
          N = vec3(0, 0, 1);
      }

      outNormalFront = N;
    }

    if (u_frontAndBackNormals == 0) return;

    // Calc back normal
    {
      vec3 position   = uvToEye(vec2(x, y), depth.g);
      vec3 positionxp = uvToEye(vec2(xp, y), depthxp.g);
      vec3 positionxn = uvToEye(vec2(xn, y), depthxn.g);
      vec3 dxl        = position - positionxn;
      vec3 dxr        = positionxp - position;

      vec3 dx = (abs(dxr.z) < abs(dxl.z)) ? dxr : dxl;

      vec3 positionyp = uvToEye(vec2(x, yp), depthyp.g);
      vec3 positionyn = uvToEye(vec2(x, yn), depthyn.g);
      vec3 dyb        = position - positionyn;
      vec3 dyt        = positionyp - position;

      vec3 dy = (abs(dyt.z) < abs(dyb.z)) ? dyt : dyb;

      //Compute Gradients of Depth and Cross Product Them to Get Normal
      vec3 N = normalize(cross(dx, dy));
      if(isnan(N.x) || isnan(N.y) || isnan(N.y) ||
         isinf(N.x) || isinf(N.y) || isinf(N.z)) {
          N = vec3(0, 0, 1);
      }

      outNormalBack = N;
    }
}
