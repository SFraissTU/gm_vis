#version 450
layout(location = 0) in vec3 in_vertex;

flat out int coloridx;

uniform mat4 projMatrix;
uniform mat4 viewMatrix;

void main() {
	//Correct coordinate system
	vec3 worldpos = vec3(in_vertex.x, in_vertex.z, -in_vertex.y);
	vec4 frag_vertex = vec4(worldpos, 1.0f);
	gl_Position = projMatrix * viewMatrix  * frag_vertex;
	coloridx = gl_VertexID % 4;
}