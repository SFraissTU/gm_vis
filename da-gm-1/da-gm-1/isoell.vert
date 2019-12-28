#version 330
layout(location = 0) in vec3 in_vertex;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in mat4 in_transform;
layout(location = 3) in mat3 in_normtrans;

out vec4 frag_position;
out vec3 frag_normal;

uniform mat4 projMatrix;
uniform mat4 viewMatrix;

void main() {
	//Correct coordinate system
	vec3 worldpos = vec3(in_vertex.x, in_vertex.z, -in_vertex.y);
	frag_position = in_transform * vec4(worldpos, 1.0f);
	gl_Position = projMatrix * viewMatrix  * frag_position;
	frag_normal = in_normtrans * in_normal;
}