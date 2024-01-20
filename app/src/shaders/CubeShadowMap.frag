#version 330 core

in vec4 FragPos;

uniform vec3 LightPosition;
uniform float far_plane;

void main()
{
    float lightDistance = length(FragPos.xyz - LightPosition);
    
    lightDistance = lightDistance / far_plane;
    
    gl_FragDepth = lightDistance;
} 