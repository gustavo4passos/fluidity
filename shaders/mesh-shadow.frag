#version 450

in  vec4  fFragPos;
out float fragColor;

void main()
{
    fragColor = (fFragPos.z / fFragPos.w) * 0.5 + 0.5;
}