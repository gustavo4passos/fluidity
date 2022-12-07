#version 450 core

layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;

in flat float gSpecularSurface[];
in flat vec3  noSpecularHitPos[];

in  vec3 gEyePos[];
out vec3 fEyePos;

uniform float uNearZ;
uniform float uFarZ;

out flat float fIntensity;

void main()
{
    // if (gSpecularSurface[0] <= 0)
    // {
    //     return;
    // }

    // One of the vertices is located beyond far plane. Discard it.
    if (gEyePos[0].z <= -uFarZ ||
        gEyePos[1].z <= -uFarZ ||
        gEyePos[2].z <= -uFarZ)
    {
        EndPrimitive();
        return;
    }

    vec3 v0 = gEyePos[0];
    vec3 v1 = gEyePos[1];
    vec3 v2 = gEyePos[2];

    vec3 v0NoSpec = noSpecularHitPos[0];
    vec3 v1NoSpec = noSpecularHitPos[1];
    vec3 v2NoSpec = noSpecularHitPos[2];

    vec3 v0v1 = v1 - v0;
    vec3 v0v2 = v2 - v0;
    vec3 n = cross(v0v1, v0v2);
    float area = 0.5 * length(n);

    vec3 v0v1NoSpec = v1NoSpec - v0NoSpec;
    vec3 v0v2NoSpec = v2NoSpec - v0NoSpec;
    vec3 nNoSpec = cross(v0v1NoSpec, v0v2NoSpec);
    float areaNoSpec = 0.5 * length(nNoSpec);

    gl_Position = gl_in[0].gl_Position;
    fEyePos = gEyePos[0];
    fIntensity = areaNoSpec / area;
    EmitVertex();

    gl_Position = gl_in[1].gl_Position;
    fEyePos = gEyePos[1];
    fIntensity = areaNoSpec / area;
    EmitVertex();

    gl_Position = gl_in[2].gl_Position;
    fEyePos = gEyePos[2];
    fIntensity = areaNoSpec / area;
    EmitVertex();

    EndPrimitive();
}
