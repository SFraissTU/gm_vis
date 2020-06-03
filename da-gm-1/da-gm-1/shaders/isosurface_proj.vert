#version 450

//Isosurface Visualization
//Vertex Shader. Projects ellipsoids.

layout(location = 0) in vec3 in_vertex;
layout(location = 1) in mat4 in_transform;

out int gaussIndex;
out float depthValue;
out vec3 position;

uniform mat4 projMatrix;
uniform mat4 viewMatrix;

void main() {
	//Correct coordinate system
	vec4 frag_position = in_transform * vec4(in_vertex, 1.0f);
	frag_position = vec4(frag_position.x, frag_position.z, -frag_position.y, frag_position.w);
	vec4 view_position = viewMatrix  * frag_position;
	gl_Position = projMatrix * view_position;
	gaussIndex = gl_InstanceID;
	//gaussIndex = int(frag_position.y) % 10;
	//gaussIndex = frag_position.y / 6;
	depthValue = length(view_position.xyz);
	position = frag_position.xzy;
}