#version 450

layout(location = 0) in vec3 vPos;

out vec3 fTexCoords;

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
    fTexCoords = vPos;
    mat4 viewMatrixWithoutTranslation = mat4(mat3(viewMatrix));
    gl_Position = projectionMatrix * viewMatrixWithoutTranslation * vec4(vPos, 1.0);
    gl_Position = gl_Position.xyww;
}