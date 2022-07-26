#version 330

flat in int coloridx;

out vec4 fragColor;

uniform float opacity;

void main() {
	if (coloridx == 0) {
		fragColor = vec4(1.0f, 0.0f, 0.0f, opacity);
	}
	if (coloridx == 1) {
		fragColor = vec4(0.0f, 1.0f, 0.0f, opacity);
	}
	if (coloridx == 2) {
		fragColor = vec4(0.0f, 0.0f, 1.0f, opacity);
	}
	if (coloridx == 3) {
		fragColor = vec4(1.0f, 1.0f, 1.0f, opacity);
	}
}