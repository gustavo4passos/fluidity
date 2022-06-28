#version 450

in vec2 f_TexCoord;
out vec4 fragmentColor;
uniform sampler2D tex;

void main()
{
    vec4 texColor = texture(tex, f_TexCoord);
    // fragmentColor = vec4(texColor.r, 0, 0, 1.0);
    // fragmentColor = vec4(vec3(texColor.r - 0.7), 1.0);
    fragmentColor = texColor;
    // if(texColora > 0 && texColor.g < 0) fragmentColor.b = -texColor.g * 0.1;
}
