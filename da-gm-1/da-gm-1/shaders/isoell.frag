#version 330
in vec4 frag_position;
in vec3 frag_normal;
in vec3 frag_scolor;

out vec3 frag_color;

uniform mat4 viewMatrix;
uniform vec3 lightDir;
uniform vec4 surfaceColor;

void main() {
	vec3 normal = normalize(frag_normal);
	vec3 l = -normalize(lightDir);
	vec3 eyePos = -viewMatrix[3].xyz;
	vec3 v = normalize(eyePos - frag_position.xyz);
	vec3 r = reflect(-l, normal);
	float nl = dot(normal, l);
	//frag_color = surfaceColor.rgb * (0.3 + max(nl, 0.0f)*0.7);
	frag_color = frag_scolor * (0.3 + max(nl, 0.0f)*0.7);
	if (nl > 0) {
		frag_color += 0.3*pow(max(dot(r,v),0), 0.8f);
	}
}