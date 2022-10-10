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
// fragment shader, modified gaussian filter
#version 410 core

#define FIX_OTHER_WEIGHT
#define RANGE_EXTENSION

#define PI_OVER_8 0.392699082f

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
uniform float     u_ParticleRadius;
uniform int       u_FilterSize;
uniform int       u_MaxFilterSize;
uniform int       u_ScreenWidth;
uniform int       u_ScreenHeight;

// u_DoFilter1D = 1, 0, and -1 (-1 mean filter2D with fixed radius)
uniform int u_DoFilter1D;
uniform int u_FilterDirection;

in vec2   f_TexCoord;
out vec3  outDepth;

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
const int   fixedFilterRadius = 4;
const float thresholdRatio    = 10.5;
const float clampRatio        = 1;

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
float compute_weight1D(float r, float two_sigma2)
{
    return exp(-r * r / two_sigma2);
}

float compute_weight2D(vec2 r, float two_sigma2)
{
    return exp(-dot(r, r) / two_sigma2);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void ModifiedGaussianFilter1D(inout float sampleDepth, inout float weight, inout float weight_other, inout float upper, inout float lower, float lower_clamp, float threshold)
{
    if(sampleDepth > upper) {
        weight = 0;
#ifdef FIX_OTHER_WEIGHT
        weight_other = 0;
#endif
    } else {
        if(sampleDepth < lower) {
            sampleDepth = lower_clamp;
        }
#ifdef RANGE_EXTENSION
        else {
            upper = max(upper, sampleDepth + threshold);
            lower = min(lower, sampleDepth - threshold);
        }
#endif
    }
}

void ModifiedGaussianFilter2D(inout float sampleDepth, inout float weight, inout float weight_other, inout float upper, inout float lower, float lower_clamp, float threshold)
{
    if(sampleDepth > upper) {
        weight = 0;
#ifdef FIX_OTHER_WEIGHT
        weight_other = 0;
#endif
    } else {
        if(sampleDepth < lower) {
            sampleDepth = lower_clamp;
        }
#ifdef RANGE_EXTENSION
        else {
            upper = max(upper, sampleDepth + threshold);
            lower = min(lower, sampleDepth - threshold);
        }
#endif
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
vec2 filter1D(vec2 pixelDepth)
{
    if(u_FilterSize == 0) {
        return pixelDepth;
    }

    // Depth indepentent parameters
    vec2  blurRadius = vec2(1.0 / u_ScreenWidth, 1.0 / u_ScreenHeight);
    float threshold  = u_ParticleRadius * thresholdRatio;
    float ratio      = u_ScreenHeight / 2.0 / tan(PI_OVER_8);
    float K          = -u_FilterSize * ratio * u_ParticleRadius * 0.1f;
    vec4  dtc        = (u_FilterDirection == 0) ? vec4(blurRadius.x, 0, -blurRadius.x, 0) : vec4(0, blurRadius.y, 0, -blurRadius.y);

    ivec2 filterSize = ivec2(min(u_MaxFilterSize, int(ceil(K / pixelDepth.r))), min(u_MaxFilterSize, int(ceil(K / pixelDepth.g))));
    vec2  sigma      = filterSize / 3.0f;
    vec2  two_sigma2 = 2.0f * sigma * sigma;

    vec4  f_tex = f_TexCoord.xyxy;
    float r     = 0;
    float dr    = dtc.x + dtc.y;

    vec2 upper       = pixelDepth + vec2(threshold);
    vec2 lower       = pixelDepth - vec2(threshold);
    vec2 lower_clamp = pixelDepth - vec2(u_ParticleRadius * clampRatio);

    vec2 sum2r  = vec2(pixelDepth.r, 0);
    vec2 wsum2r = vec2(1, 0);

    vec2 sum2g  = vec2(pixelDepth.g, 0);
    vec2 wsum2g = vec2(1, 0);

    vec2 upper1 = upper;
    vec2 upper2 = upper;
    vec2 lower1 = lower;
    vec2 lower2 = lower;
    vec4 sampleDepth;
    vec2 w2r;
    vec2 w2g;

    for(int x = 1; x <= filterSize.r; ++x) {
        f_tex += dtc;
        r     += dr;

        sampleDepth.xy = texture(u_DepthTex, f_tex.xy).rg;
        sampleDepth.zw = texture(u_DepthTex, f_tex.zw).rg;

        // Front face
        w2r = vec2(compute_weight1D(r, two_sigma2.r));
        ModifiedGaussianFilter1D(sampleDepth.x, w2r.x, w2r.y, upper1.r, lower1.r, lower_clamp.r, threshold);
        ModifiedGaussianFilter1D(sampleDepth.z, w2r.y, w2r.x, upper2.r, lower2.r, lower_clamp.r, threshold);

        sum2r  += sampleDepth.xz * w2r;
        wsum2r += w2r;

        // Back face
        w2g = vec2(compute_weight1D(r, two_sigma2.g));
        ModifiedGaussianFilter1D(sampleDepth.y, w2g.x, w2g.y, upper1.g, lower1.g, lower_clamp.g, threshold);
        ModifiedGaussianFilter1D(sampleDepth.w, w2g.y, w2g.x, upper2.g, lower2.g, lower_clamp.g, threshold);

        sum2g  += sampleDepth.yw * w2g;
        wsum2g += w2g;
    }

    vec2 filterValR = vec2(sum2r.x, wsum2r.x) + vec2(sum2r.y, wsum2r.y);
    vec2 filterValG = vec2(sum2g.x, wsum2g.x) + vec2(sum2g.y, wsum2g.y);
    return vec2(filterValR.x / filterValR.y, filterValG.x / filterValG.y);
}

float filter2D(float pixelDepth)
{
    if(u_FilterSize == 0) {
        return pixelDepth;
    }

    vec2  blurRadius = vec2(1.0 / u_ScreenWidth, 1.0 / u_ScreenHeight);
    float threshold  = u_ParticleRadius * thresholdRatio;
    float ratio      = u_ScreenHeight / 2.0 / tan(PI_OVER_8);
    float K          = -u_FilterSize * ratio * u_ParticleRadius * 0.1f;
    int   filterSize = (u_DoFilter1D < 0) ? fixedFilterRadius : min(u_MaxFilterSize, int(ceil(K / pixelDepth)));

    float upper       = pixelDepth + threshold;
    float lower       = pixelDepth - threshold;
    float lower_clamp = pixelDepth - u_ParticleRadius * clampRatio;

    float sigma      = filterSize / 3.0f;
    float two_sigma2 = 2.0f * sigma * sigma;

    vec4 f_tex = f_TexCoord.xyxy;

    vec2 r     = vec2(0, 0);
    vec4 sum4  = vec4(pixelDepth, 0, 0, 0);
    vec4 wsum4 = vec4(1, 0, 0, 0);
    vec4 sampleDepth;
    vec4 w4;

    for(int x = 1; x <= filterSize; ++x) {
        r.x     += blurRadius.x;
        f_tex.x += blurRadius.x;
        f_tex.z -= blurRadius.x;
        vec4 f_tex1 = f_tex.xyxy;
        vec4 f_tex2 = f_tex.zwzw;

        for(int y = 1; y <= filterSize; ++y) {
            f_tex1.y += blurRadius.y;
            f_tex1.w -= blurRadius.y;
            f_tex2.y += blurRadius.y;
            f_tex2.w -= blurRadius.y;

            sampleDepth.x = texture(u_DepthTex, f_tex1.xy).r;
            sampleDepth.y = texture(u_DepthTex, f_tex1.zw).r;
            sampleDepth.z = texture(u_DepthTex, f_tex2.xy).r;
            sampleDepth.w = texture(u_DepthTex, f_tex2.zw).r;

            r.y += blurRadius.y;
            w4   = vec4(compute_weight2D(blurRadius * r, two_sigma2));

            ModifiedGaussianFilter2D(sampleDepth.x, w4.x, w4.w, upper, lower, lower_clamp, threshold);
            ModifiedGaussianFilter2D(sampleDepth.y, w4.y, w4.z, upper, lower, lower_clamp, threshold);
            ModifiedGaussianFilter2D(sampleDepth.z, w4.z, w4.y, upper, lower, lower_clamp, threshold);
            ModifiedGaussianFilter2D(sampleDepth.w, w4.w, w4.x, upper, lower, lower_clamp, threshold);

            sum4  += sampleDepth * w4;
            wsum4 += w4;
        }
    }

    vec2 filterVal;
    filterVal.x = dot(sum4, vec4(1, 1, 1, 1));
    filterVal.y = dot(wsum4, vec4(1, 1, 1, 1));
    return filterVal.x / filterVal.y;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void main()
{
    vec4 pixelDepth = texture(u_DepthTex, f_TexCoord);

    if(pixelDepth.b < 0) {
        outDepth = pixelDepth.rgb;
    } else {
        outDepth = (u_DoFilter1D == 1) ? vec3(filter1D(pixelDepth.rg), 1.0) : vec3(filter2D(pixelDepth.r), pixelDepth.g, pixelDepth.b);
    }
}