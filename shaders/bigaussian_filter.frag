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

const int filterSize = 3;
const int maxFilterSize = 3;

const float thresholdRatio = 10.0;
float compute_weight2D(vec2 r, float two_sigma2)
{
    return exp(-dot(r, r) / two_sigma2);
}

float compute_weight1D(float r, float two_sigma2)
{
    return exp(-r * r / two_sigma2);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void main()
{
    vec2 blurRadius = vec2(1.0 / float(bufferWidth), 1.0 / float(bufferHeight));

    float pixelDepth = texture(workingSurface, texCoord).r;
    float finalDepth;

    if(pixelDepth >= 0.0f || pixelDepth < -1000.0f) {
        finalDepth = pixelDepth;
    } else {
        float ratio      = bufferHeight / 2.0 / tan(45.0 / 2.0);
        float K          = -filterSize * ratio * pointRadius * 0.1f;
        int   filterSize = min(maxFilterSize, int(ceil(K / pixelDepth)));
        float sigma      = filterSize / 3.0f;
        float two_sigma2 = 2.0f * sigma * sigma;

        float threshold       = pointRadius * thresholdRatio;
        float sigmaDepth      = threshold / 3.0f;
        float two_sigmaDepth2 = 2.0f * sigmaDepth * sigmaDepth;

        vec4 f_tex = texCoord.xyxy;
        vec2 r     = vec2(0, 0);
        vec4 sum4  = vec4(pixelDepth, 0, 0, 0);
        vec4 wsum4 = vec4(1, 0, 0, 0);
        vec4 sampleDepth;
        vec4 w4_r;
        vec4 w4_depth;
        vec4 rDepth;

        for(int x = 1; x <= filterSize; ++x) {
            r.x     += blurRadius.x;
            f_tex.x += blurRadius.x;
            f_tex.z -= blurRadius.x;
            vec4 f_tex1 = f_tex.xyxy;
            vec4 f_tex2 = f_tex.zwzw;

            for(int y = 1; y <= filterSize; ++y) {
                r.y += blurRadius.y;

                f_tex1.y += blurRadius.y;
                f_tex1.w -= blurRadius.y;
                f_tex2.y += blurRadius.y;
                f_tex2.w -= blurRadius.y;

                sampleDepth.x = texture(workingSurface, f_tex1.xy).r;
                sampleDepth.y = texture(workingSurface, f_tex1.zw).r;
                sampleDepth.z = texture(workingSurface, f_tex2.xy).r;
                sampleDepth.w = texture(workingSurface, f_tex2.zw).r;

                rDepth     = sampleDepth - vec4(pixelDepth);
                w4_r       = vec4(compute_weight2D(blurRadius * r, two_sigma2));
                w4_depth.x = compute_weight1D(rDepth.x, two_sigmaDepth2);
                w4_depth.y = compute_weight1D(rDepth.y, two_sigmaDepth2);
                w4_depth.z = compute_weight1D(rDepth.z, two_sigmaDepth2);
                w4_depth.w = compute_weight1D(rDepth.w, two_sigmaDepth2);

                sum4  += sampleDepth * w4_r * w4_depth;
                wsum4 += w4_r * w4_depth;
            }
        }

        vec2 filterVal;
        filterVal.x = dot(sum4, vec4(1, 1, 1, 1));
        filterVal.y = dot(wsum4, vec4(1, 1, 1, 1));

        finalDepth = filterVal.x / filterVal.y;
    }

    smoothedSurface = vec4(vec3(finalDepth), 1.0);
}