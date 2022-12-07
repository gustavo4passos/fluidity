#version 450

in  vec4  fFragPos;
in  vec4  fFragEyePos;

out float fragScreenPos;
out float fragEyePos;

void main()
{
    fragScreenPos = (fFragPos.z / fFragPos.w) * 0.5 + 0.5;
    fragEyePos = fFragEyePos.z;
}
