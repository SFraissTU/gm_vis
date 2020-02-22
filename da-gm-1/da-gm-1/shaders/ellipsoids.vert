#version 330
layout(location = 0) in vec3 in_vertex;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in mat4 in_transform;
layout(location = 6) in mat4 in_normtrans;

out vec4 frag_position;
out vec3 frag_normal;

uniform mat4 projMatrix;
uniform mat4 viewMatrix;

void main() {
	//Correct coordinate system
	frag_position = in_transform * vec4(in_vertex, 1.0f);
	frag_position = vec4(frag_position.x, frag_position.z, -frag_position.y, frag_position.w);
	gl_Position = projMatrix * viewMatrix  * frag_position;
	frag_normal = mat3(in_normtrans) * in_normal;
	frag_normal = vec3(frag_normal.x, frag_normal.z, -frag_normal.y);
}