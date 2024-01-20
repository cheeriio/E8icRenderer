#version 330 core

layout(location = 0) in vec3 vertexPosition;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

void main(){	
	gl_Position =  P * V * M * vec4(vertexPosition, 1);
}
