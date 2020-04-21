#version 330
layout(location = 0) in vec4 in_position;
layout(location = 1) in float in_color;

out vec3 frag_color;
flat out int frag_index;

uniform bool useInColor;
uniform mat4 projMatrix;
uniform mat4 viewMatrix;
uniform vec4 surfaceColor;
uniform sampler1D transferTex;

void main() {
	//Correct coordinate system
	vec4 frag_position = vec4(in_position.x, in_position.z, -in_position.y, in_position.w);
	gl_Position = projMatrix * viewMatrix  * frag_position;
	if (useInColor) {
		frag_color = texture(transferTex, 0.25 + 0.75*in_color).rgb;
	} else {
		frag_color = surfaceColor.rgb;
	}
	//frag_color = vec3(1, -frag_position.z / 5.0, 0);
	frag_index = gl_VertexID;
}