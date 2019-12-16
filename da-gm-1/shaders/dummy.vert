#version 330
layout(location = 0) in vec3 in_vertex;

out vec4 frag_vertex;

uniform mat4 projMatrix;
uniform mat4 viewMatrix;

void main() {
	frag_vertex = vec4(in_vertex, 1.0f);
	gl_Position = projMatrix * viewMatrix  * frag_vertex;
}