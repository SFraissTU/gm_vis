#version 450

// Accelerated Density Visualization
// Fragment Shader. Calcualtes contribution to pixel sum

flat in int gaussIndex;

out float frag_sum;

layout(location = 1) uniform mat4 viewMatrix;
layout(location = 2) uniform mat4 invViewMatrix;
layout(location = 3) uniform int width;
layout(location = 4) uniform int height;
layout(location = 5) uniform float fov;
layout(location = 6) uniform sampler1D erfTex;
layout(location = 7) uniform float kappa;
layout(location = 8) uniform float far;

float sqrt2pi = 2.506628275;

struct Gaussian {
	vec4 mu_alpha;
	mat4 invsigma;
};

layout(std430, binding=0) buffer GaussianMixture {
	Gaussian gaussians[];
} mixture;

float evalGaussian(vec3 rorig, vec3 rdir, Gaussian gauss) {
	mat3 inv = mat3(gauss.invsigma);
	vec3 rs = rdir*inv;
	vec3 xm = rorig - gauss.mu_alpha.xyz;

	float C_3 = dot(rs, (rorig - gauss.mu_alpha.xyz)) + kappa;
	float C_2 = 0.5 * dot(rs, rdir);
	float C_1 = 0.5 * dot(xm * inv, xm) - kappa * far;
	//float C_0 = gauss.mu_alpha.w * exp((C_3 * C_3)/(4*C_2) - C_1);
	float mu_k = dot(rs, (gauss.mu_alpha.xyz - rorig)) / dot(rs, rdir);
	vec3 pivec = rorig + mu_k*rdir - gauss.mu_alpha.xyz;
	//float C_0 = gauss.mu_alpha.w * exp(-0.5*dot(pivec*inv, pivec)) * exp(sigma*far);
	float C_0 = sqrt2pi * gauss.mu_alpha.w * sqrt(1 / dot(rs, rdir)) * exp(-0.5*dot(pivec*inv, pivec));// * exp(sigma*far);

	float valB = (C_3 + 2*C_2*far) / (2 * sqrt(C_2));
	float valA = C_3 / (2 * sqrt(C_2));
	float res = C_0 * (texture(erfTex, valB * 0.1 + 0.5).r  - texture(erfTex, valA * 0.1 + 0.5).r);
	return res;
	//return gauss.mu_alpha.w * sqrt(1 / dot(rs, rdir)) * exp(-0.5*dot(pivec*inv, pivec));
}


void main() {
	vec2 pixel_center = gl_FragCoord.xy;
	vec2 pixel_uv = pixel_center / vec2(width, height);
	vec2 d = pixel_uv * 2.0 - 1.0;
	float aspectRatio = float(width) / float(height);	//or just pass aspect ratio

	vec3 origin = vec3(0, 0, 0.0);
	vec3 direction = normalize(vec3(d.x * aspectRatio, d.y, -1/tan(fov/2.0)));	//or just pass -1/tan(fov/2.0)
	vec4 p1 = vec4(origin, 1.0);
	vec4 p2 = vec4(origin + direction, 1.0);
	vec4 vp1 = invViewMatrix * p1;
	vec4 vp2 = invViewMatrix * p2;
	origin = vec3(vp1);
	direction = vec3(normalize(vp2 - vp1));

	origin = vec3(origin.x, -origin.z, origin.y);
	direction = vec3(direction.x, -direction.z, direction.y);

	frag_sum = evalGaussian(origin, direction, mixture.gaussians[gaussIndex]);
}