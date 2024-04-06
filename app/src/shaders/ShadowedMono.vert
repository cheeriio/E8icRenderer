#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 2) in vec3 vertexNormal_modelspace;

out vec3 Position_worldspace;
out vec3 Normal_cameraspace;
out vec3 EyeDirection_cameraspace;
out vec3 LightDirection_cameraspace;
out vec3 Normal;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform vec3 LightPosition;

void main(){
    mat3 normal_matrix = transpose(inverse(mat3(M)));

	gl_Position =  P * V * M * vec4(vertexPosition_modelspace, 1);
    Position_worldspace = (M * vec4(vertexPosition_modelspace, 1)).xyz;
    Normal_cameraspace = ( V * mat4(normal_matrix) * vec4(vertexNormal_modelspace, 0)).xyz;

    vec3 vertexPosition_cameraspace = (V * M * vec4(vertexPosition_modelspace, 1)).xyz;
    EyeDirection_cameraspace = vec3(0,0,0) - vertexPosition_cameraspace;

    vec3 LightPosition_cameraspace = (V * vec4(LightPosition, 1)).xyz;
    LightDirection_cameraspace = LightPosition_cameraspace + EyeDirection_cameraspace;

    Normal = transpose(inverse(mat3(M))) * vertexNormal_modelspace;
}
