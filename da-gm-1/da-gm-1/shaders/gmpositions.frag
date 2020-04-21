#version 450
in vec3 frag_color;
flat in int frag_index;

layout(location = 0) out vec3 out_color;
layout(location = 1) out float out_index;

void main() {
	out_color = frag_color.rgb;
	out_index = float(frag_index) + 1;
}