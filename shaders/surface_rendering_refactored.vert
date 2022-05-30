#version 450 core

layout (location = 0) in vec3 particlePos;
out vec4 vParticlePos;

uniform float u_PointRadius;
// uniform float scale;

uniform int u_ScreenWidth;
uniform int u_ScreenHeight;

out vec3 viewCenter;

layout(std140) uniform CameraData
{
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 invViewMatrix;
    mat4 invProjectionMatrix;
    mat4 shadowMatrix;
    vec4 camPosition;
};

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

layout(std140) uniform Material
{
    vec4  ambient;
    vec4  diffuse;
    vec4  specular;
    float shininess;
} material;

const mat4 D = mat4(1., 0., 0., 0.,
                    0., 1., 0., 0.,
                    0., 0., 1., 0.,
                    0., 0., 0., -1.);

void ComputePointSizeAndPosition(mat4 T)
{
    vec2 xbc;
    vec2 ybc;

    mat4  R = transpose(projectionMatrix * viewMatrix * T);
    float A = dot(R[ 3 ], D * R[ 3 ]);
    float B = -2. * dot(R[ 0 ], D * R[ 3 ]);
    float C = dot(R[ 0 ], D * R[ 0 ]);
    xbc[ 0 ] = (-B - sqrt(B * B - 4. * A * C)) / (2.0 * A);
    xbc[ 1 ] = (-B + sqrt(B * B - 4. * A * C)) / (2.0 * A);
    float sx = abs(xbc[ 0 ] - xbc[ 1 ]) * .5 * u_ScreenWidth;

    A        = dot(R[ 3 ], D * R[ 3 ]);
    B        = -2. * dot(R[ 1 ], D * R[ 3 ]);
    C        = dot(R[ 1 ], D * R[ 1 ]);
    ybc[ 0 ] = (-B - sqrt(B * B - 4. * A * C)) / (2.0 * A);
    ybc[ 1 ] = (-B + sqrt(B * B - 4. * A * C)) / (2.0 * A);
    float sy = abs(ybc[ 0 ] - ybc[ 1 ]) * .5 * u_ScreenHeight;

    float pointSize = ceil(max(sx, sy));
    gl_PointSize = pointSize;
}


void main() 
{
    // float dist = length(particlePos.xyz);
    // gl_PointSize = radius * (scale / dist);
    mat4 T = mat4(u_PointRadius, 0, 0, 0,
                  0, u_PointRadius, 0, 0,
                  0, 0, u_PointRadius, 0,
                  vParticlePos.x, vParticlePos.y, vParticlePos.z, 1.0);
    ComputePointSizeAndPosition(T);

    // gl_PointSize = u_PointRadius;
    gl_Position =  projectionMatrix * viewMatrix * vec4(particlePos.xyz, 1.0);
    vParticlePos = gl_Position;

    // Calc view center
    viewCenter = (viewMatrix * vec4(particlePos.xyz, 1.0)).xyz;
}
