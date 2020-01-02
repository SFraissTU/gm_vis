#version 330
in vec4 frag_position;
in vec3 frag_normal;

out vec3 frag_color;

uniform vec3 lightDir;
//uniform vec3 eyePos;
uniform vec4 surfaceColor;

void main() {
	vec3 normal = normalize(frag_normal);
	vec3 l = -lightDir;
	frag_color = surfaceColor.rgb * (0.3 + max(dot(normal, l), 0.0f)*0.7);
}