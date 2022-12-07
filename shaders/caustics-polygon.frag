#version 450 core

in vec2 fTexCoord;
in vec3 fEyePos;
in flat float fIntensity;
out vec4 fragColor;

void main()
{
  if (fEyePos.z < -10000) discard;
  fragColor = vec4(fIntensity);
  fragColor.a = 1.0;
}