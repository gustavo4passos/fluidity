#version 450

in vec2 texCoord;
out vec4 fragmentColor;
uniform sampler2D tex;
uniform int gammaCorrectionEnabled;

void main()
{
    vec4 texColor = texture(tex, texCoord);
    // fragmentColor = vec4(texColor.r * 10, 0, 0, 1.0);
    // fragmentColor = vec4(vec3(-texColor.r / 13.0), 1.0);
    // fragmentColor = texColor;
    // if(texColora > 0 && texColor.g < 0) fragmentColor.b = -texColor.g * 0.1;
    vec3 preGammaCorrectionColor = texColor.xyz;
    if (gammaCorrectionEnabled == 1)
    {
        float gamma = 2.2;
        fragmentColor = vec4(pow(preGammaCorrectionColor, vec3(1/gamma)), 1.0);
    }
    else fragmentColor = vec4(preGammaCorrectionColor, 1.0);
}
