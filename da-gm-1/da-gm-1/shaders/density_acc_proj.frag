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
layout(location = 6) uniform sampler1D gaussTex;
layout(location = 7) uniform float kappa;

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
	float sig2 = 1.0 / dot(rs, rdir);
	float mu = dot(rs, gauss.mu_alpha.xyz-rorig) * sig2;
	float sig = sqrt(sig2);
	vec3 pivec = rorig + mu*rdir - gauss.mu_alpha.xyz;
	float omega = sqrt2pi * gauss.mu_alpha.w * sig * exp(-0.5*dot(pivec*inv, pivec));
	//float gammak = sqrt2pi * gauss.mu_alpha.w * sig * exp(-0.5*dot(pivec*inv, pivec));
	//return gammak * texture(gaussTex, (mu / sig) * 0.1 + 0.5).r;
	//Homogenous falloff
	//float gamma = exp(0.5*kappa*(sig2*kappa-2*mu));
	//return omega * gamma * texture(gaussTex, (mu / sig - sig*kappa) * 0.1 + 0.5).r;
	//Heterogenous falloff
	float phi = texture(gaussTex, (mu / sig) * 0.1 + 0.5).r;
	if (kappa == 0)
	{
		return omega * phi;
	}
	//everything else is rubbish!
	else
	{
		//VARIANTE: kappa(t) = kappa*d
		//return (1 / kappa) * exp(-kappa*omega)*(1-exp(-kappa*omega*phi));
		//[OBSOLETE] VARIANTE: kappa(t) = kappa*log(d)
		//if (kappa == 1)
		//{
			//return log(2+phi)-log(2);
		//}
		//else
		//{
			//return (2*pow(omega, 1-kappa)*(pow(1+phi, 1-kappa)-1))/(1-kappa);
		//}
		//VARIANTE: kappa(t) = kappa*log(d+1)
		if (kappa == 1)
		{
			return log(abs(2*omega*(1+phi)-2)) - log(abs(2*omega-2));
		}
		else
		{
			return (pow(omega + omega*phi + 1, 1-kappa) - pow(omega + 1, 1-kappa))/(1-kappa);
		}
	}
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