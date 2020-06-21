#version 330
in vec4 frag_position;
in vec3 frag_normal;
in vec3 frag_color;
flat in int frag_index;

layout(location = 0) out vec3 out_color;
layout(location = 1) out float out_index;

uniform mat4 viewMatrix;
uniform vec3 eyePos;
uniform vec3 lightDir;

void main() {
	vec3 normal = normalize(frag_normal);
	vec3 l = -normalize(lightDir);
	vec3 v = normalize(eyePos - frag_position.xyz);
	vec3 r = reflect(-l, normal);
	float nl = dot(normal, l);
	out_color = frag_color * (0.3 + max(nl, 0.0f)*0.8);
	if (nl > 0) {
		out_color += 0.3*pow(max(dot(r,v),0), 1.5f);
	}
	out_index = frag_index + 1;
}