#version 450 core

layout (location = 0) out vec4 frontSurface;
in vec4 vParticlePos;
in vec3 viewCenter;

uniform float pointRadius;

layout(std140) uniform CameraData
{
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 invViewMatrix;
    mat4 invProjectionMatrix;
    mat4 shadowMatrix;
    vec4 camPosition;
};

void main() 
{
    vec3 viewDir = normalize(viewCenter);

    vec3 relativePos;
    relativePos.xy = gl_PointCoord.st;
    relativePos.xy = relativePos.xy * vec2(2, -2) + vec2(-1, 1);

    float magnitude = dot(relativePos.xy, relativePos.xy);
    if(magnitude > 1.0) discard;

    vec3 normal;
    normal.xy = relativePos.xy;
    normal.z = sqrt(1.0 - magnitude);

    // float frontSurfaceDepth = sqrt(1.0 - magnitude);
    vec3 fragPos = viewCenter + normal * pointRadius;
    float fragDepth = (projectionMatrix  * vec4(fragPos, 1.0)).z;

    frontSurface = vec4(fragDepth, -fragDepth, 0.0, 1.0);
    frontSurface = vec4(1.0);
}
