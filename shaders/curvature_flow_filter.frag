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

const float thresholdRatio = 5.0;

vec3 meanCurvature(vec2 pos)
{
    // Width of one pixel
    vec2 dx = vec2(1.0f / bufferWidth, 0.0f);
    vec2 dy = vec2(0.0f, 1.0f / bufferHeight);

    // Central z value
    float zc = texture(workingSurface, pos).r - 1.0f;

    float zdxp = texture(workingSurface, pos + dx).r - 1.0f;
    float zdxn = texture(workingSurface, pos - dx).r - 1.0f;

    float zdx = 0.5f * (zdxp - zdxn);

    float zdyp = texture(workingSurface, pos + dy).r - 1.0f;
    float zdyn = texture(workingSurface, pos - dy).r - 1.0f;

    float zdy = 0.5f * (zdyp - zdyn);

    // Take second order finite differences
    float zdx2 = zdxp + zdxn - 2.0f * zc;
    float zdy2 = zdyp + zdyn - 2.0f * zc;

    // Second order finite differences, alternating variables
    float zdxpyp = texture(workingSurface, pos + dx + dy).r - 1.0f;
    float zdxnyn = texture(workingSurface, pos - dx - dy).r - 1.0f;
    float zdxpyn = texture(workingSurface, pos + dx - dy).r - 1.0f;
    float zdxnyp = texture(workingSurface, pos - dx + dy).r - 1.0f;

    float zdxy = (zdxpyp + zdxnyn - zdxpyn - zdxnyp) / 4.0f;

    if(abs(zdx) > pointRadius * thresholdRatio) {
        zdx  = 0.0f;
        zdx2 = 0.0f;
    }

    if(abs(zdy) > pointRadius * thresholdRatio) {
        zdy  = 0.0f;
        zdy2 = 0.0f;
    }

    if(abs(zdxy) > pointRadius * thresholdRatio) {
        zdxy = 0.0f;
    }

    // Projection transform inversion terms
    float cx = 2.0f / (bufferWidth * -projection[0][0]);
    float cy = 2.0f / (bufferHeight * -projection[1][1]);

    // Normalization term
    float d = cy * cy * zdx * zdx + cx * cx * zdy * zdy + cx * cx * cy * cy * zc * zc;

    // Derivatives of said term
    float ddx = cy * cy * 2.0f * zdx * zdx2 + cx * cx * 2.0f * zdy * zdxy + cx * cx * cy * cy * 2.0f * zc * zdx;
    float ddy = cy * cy * 2.0f * zdx * zdxy + cx * cx * 2.0f * zdy * zdy2 + cx * cx * cy * cy * 2.0f * zc * zdy;

    // Temporary variables to calculate mean curvature
    float ex = 0.5f * zdx * ddx - zdx2 * d;
    float ey = 0.5f * zdy * ddy - zdy2 * d;

    // Finally, mean curvature
    float h = 0.5f * ((cy * ex + cx * ey) / pow(d, 1.5f));

    return (vec3(zdx, zdy, h));
}


void main()
{
    // vec2 ts = textureSize(workingSurface, 0);

    // for(int i = -kernelRadius; i <= kernelRadius; i++)
    // {
    //     for(int j = -kernelRadius; j <= kernelRadius; j++)
    //     {
    //         vec4 texel = getValidTexel(
    //             workingSurface,
    //             texCoord + vec2(i, j) * (1.0 / ts));
    //         smoothedSurface += texel;
    //     }  
    // }

    // smoothedSurface /= (kernelRadius * kernelRadius * 4);

    float particleDepth = texture(workingSurface, texCoord).r;

    vec2 ts = vec2(bufferWidth, bufferHeight);
    float c = 0;
    for (int i = -1; i <= 1; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            float d = texture(workingSurface, texCoord + vec2(i, j) * (1.0 / ts)).r;
            c += d;
        }
    }

    c /= 9;

    smoothedSurface = vec4(vec3(c), 1.0);

    // if (particleDepth < -1000.0f || particleDepth > 0) {
    //     smoothedSurface = vec4(vec3(particleDepth), 1.0);
    // } 
    // else {
    //     const float dt = 0.0003f;
    //     const float dzt = 1000.0f;
    //     vec3  dxyz = meanCurvature(texCoord);
        
    //     float filteredDepth = particleDepth + dxyz.z * dt * (1.0f + (abs(dxyz.x) + abs(dxyz.y)) * dzt);
    //     smoothedSurface = vec4(vec3(filteredDepth), 1.0);
    // }
}