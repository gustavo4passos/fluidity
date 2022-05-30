#version 450 core

uniform sampler2D workingSurface;
uniform int kernelRadius;
layout (location = 0) out vec4 smoothedSurface;

in vec2 texCoord;

uniform int bufferWidth;
uniform int bufferHeight;
uniform float pointRadius;

uniform mat4 projection;
uniform mat4 view;

const int   fixedFilterRadius = 4;
const float thresholdRatio    = 10.5;
const float clampRatio        = 1;

const int u_FilterSize = 4;
const int u_MaxFilterSize = 5;
const int u_FilterDirection = 0;
const int u_DoFilter1D = 0;
#define PI_OVER_8 0.392699082f

float compute_weight1D(float r, float two_sigma2)
{
    return exp(-r * r / two_sigma2);
}

float compute_weight2D(vec2 r, float two_sigma2)
{
    return exp(-dot(r, r) / two_sigma2);
}

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

float filter2D(float pixelDepth)
{
    if(u_FilterSize == 0) {
        return pixelDepth;
    }

    vec2  blurRadius = vec2(1.0 / bufferWidth, 1.0 / bufferHeight);
    float threshold  = pointRadius * thresholdRatio;
    float ratio      = bufferHeight / 2.0 / tan(PI_OVER_8);
    float K          = -u_FilterSize * ratio * pointRadius * 0.1f;
    int   filterSize = (u_DoFilter1D < 0) ? fixedFilterRadius : min(u_MaxFilterSize, int(ceil(K / pixelDepth)));

    float upper       = pixelDepth + threshold;
    float lower       = pixelDepth - threshold;
    float lower_clamp = pixelDepth - pointRadius * clampRatio;

    float sigma      = filterSize / 3.0f;
    float two_sigma2 = 2.0f * sigma * sigma;

    vec4 f_tex = texCoord.xyxy;

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

            sampleDepth.x = texture(workingSurface, f_tex1.xy).r;
            sampleDepth.y = texture(workingSurface, f_tex1.zw).r;
            sampleDepth.z = texture(workingSurface, f_tex2.xy).r;
            sampleDepth.w = texture(workingSurface, f_tex2.zw).r;

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

void main()
{
    
    vec2 ts = vec2(bufferWidth, bufferHeight);
    float c = 0;
    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            float d = texture(workingSurface, texCoord + vec2(i, j) * (1.0 / ts)).r;
            c += d;
        }
    }

    c /= 9;

    smoothedSurface = vec4(vec3(c), 1.0);

    // float pixelDepth = texture(workingSurface, texCoord).r;
    // smoothedSurface = vec4(vec3(filter2D(pixelDepth)), 1.0);
}