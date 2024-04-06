#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D Scene;
uniform sampler2D BloomBlur;

uniform float exposure;

void main()
{             
    const float gamma = 2.2;
    vec3 hdrColor = texture(Scene, TexCoords).rgb;      
    vec3 bloomColor = texture(BloomBlur, TexCoords).rgb;
    hdrColor += bloomColor;
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
    result = pow(result, vec3(1.0 / gamma));
    FragColor = vec4(result, 1.0);
}