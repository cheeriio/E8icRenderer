#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vertexNormal_modelspace;
layout(location = 3) in vec3 vertexTangent_modelspace;
layout(location = 4) in vec3 vertexBitangent_modelspace;

out vec2 UV;
out vec3 Position_worldspace;
out vec3 Normal_cameraspace;
out vec3 LightDirection_cameraspace;

out vec3 LightDirection_tangentspace;
out vec3 EyeDirection_tangentspace;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform vec3 LightPosition;

void main() {
	gl_Position =  P * V * M * vec4(vertexPosition_modelspace, 1);
	
	Position_worldspace = (M * vec4(vertexPosition_modelspace, 1)).xyz;
	
	vec3 vertexPosition_cameraspace = (V * M * vec4(vertexPosition_modelspace, 1)).xyz;
	vec3 EyeDirection_cameraspace = vec3(0,0,0) - vertexPosition_cameraspace;

	vec3 LightPosition_cameraspace = (V * vec4(LightPosition, 1)).xyz;
	LightDirection_cameraspace = LightPosition_cameraspace + EyeDirection_cameraspace;
	
	UV = vertexUV;
	mat3 normal_matrix = transpose(inverse(mat3(M)));
	vec4 vertexTangent_cameraspace = V * M * vec4(vertexTangent_modelspace, 1);
	vec4 vertexBitangent_cameraspace = V * M * vec4(vertexBitangent_modelspace, 1);
	vec4 vertexNormal_cameraspace = V * mat4(normal_matrix) * vec4(vertexNormal_modelspace, 1);
	
	mat3 TBN = transpose(mat3(
		vertexTangent_cameraspace.xyz,
		vertexBitangent_cameraspace.xyz,
		vertexNormal_cameraspace.xyz	
	));

	LightDirection_tangentspace = TBN * LightDirection_cameraspace;
	EyeDirection_tangentspace =  TBN * EyeDirection_cameraspace;
    Normal_cameraspace = ( V * mat4(normal_matrix) * vec4(vertexNormal_modelspace, 0)).xyz;

    FragPos = vec3(M * vec4(vertexPosition_modelspace, 1.0));
    Normal = transpose(inverse(mat3(M))) * vertexNormal_modelspace;
}