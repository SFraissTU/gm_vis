#version 450
in vec3 frag_color;
flat in float frag_index;

layout(location = 0) out vec3 out_color;
layout(location = 1) out float out_index;

uniform int markedGaussian;

void main() {
	if (markedGaussian == round(frag_index)) {
		out_color = vec3(1, 0, 0);
	} else {	
		out_color = frag_color.rgb;
	}
	out_index = frag_index + 1;
}