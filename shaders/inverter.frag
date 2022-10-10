#version 410 core

uniform sampler2D u_DepthTex;
in  vec2 f_TexCoord;
out vec3 invertedDepth;

void main()
{
    invertedDepth = texture(u_DepthTex, f_TexCoord).rgb;
    if (invertedDepth.b > 0) invertedDepth.g = -invertedDepth.g;
}
