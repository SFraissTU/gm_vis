#version 450

// Isosurface Visualization
// Fragment Shader. Calcualtes contribution to pixel sum

in flat int gaussIndex;
//in float depthValue;
in vec3 vPos;
in vec3 position;

out vec4 testcolor;

struct FragmentData {
	int gIndex;
	int flags;
	int next;
	int padding1;
	double z;
	double padding2;
	//ivec4 g_f_n;
	vec4 position;
};

layout(std430, binding=0) buffer FragmentList {
	FragmentData[] fragmentList;
} flb;

layout(binding=1, offset=0) uniform atomic_uint listSize;

layout(r32i) uniform iimage2D img_startidx;

uniform uint maxFragmentListLength;
uniform mat4 invViewMatrix; //ToDo: remove this
uniform int width;
uniform int height;
uniform float fov;

void main() {
	uint i = atomicCounterIncrement(listSize);
	//TODO: For some reason Fragment List is empty!
	if (i >= maxFragmentListLength) {
		atomicCounterDecrement(listSize);
		//testcolor = vec4(1.0, 0.0, 0.0, 1.0);
		discard;
		return;
	}
	flb.fragmentList[i].z = length(dvec3(vPos));
	flb.fragmentList[i].gIndex = gaussIndex;
	flb.fragmentList[i].flags = int(gl_FrontFacing);
	int oldhead = imageAtomicExchange(img_startidx, ivec2(gl_FragCoord.xy), int(i));
	//imageStore(img_startidx, ivec2(gl_FragCoord.xy * imageSize(img_startidx)), ivec4(1));
	//imageSize is wrong anyway, as this might be larger than w + h
	//imageStore(img_startidx, ivec2(gl_FragCoord.xy), ivec4(1));
	flb.fragmentList[i].next = oldhead;


//	ivec2 pixel_coords = ivec2(gl_FragCoord.xy);
//	vec2 pixel_center = vec2(pixel_coords) + vec2(0.5);
//	vec2 pixel_uv = pixel_center / vec2(width, height);
//	vec2 d = pixel_uv * 2.0 - 1.0;
//	float aspectRatio = float(width) / float(height);
//
//	vec3 origin = vec3(0, 0, 0.0);
//	vec3 direction = normalize(vec3(d.x * aspectRatio, d.y, -1/tan(fov/2.0)));
//	vec4 p1 = vec4(origin, 1.0);
//	vec4 p2 = vec4(origin + direction, 1.0);
//	vec4 vp1 = invViewMatrix * p1;
//	vec4 vp2 = invViewMatrix * p2;
//	origin = vec3(vp1);
//	direction = vec3(normalize(vp2 - vp1));
//
//	origin = vec3(origin.x, -origin.z, origin.y);
//	direction = vec3(direction.x, -direction.z, direction.y);
//
//	vec3 newdir = normalize(position - origin);
//	float newz = distance(position, origin);//newdir.length();
//	//newdir = normalize(newdir);
//	
//
//	//vec3 nposition = origin + depthValue*direction;
//	vec3 nposition = origin + newz*newdir;

	//double val = abs(position).z / 0.6;
	//val = (int(val) % 10) / 10.0;
	//testcolor = vec4(val, val, val, 1.0);

	//testcolor = vec4(depthValue / 200.0);
	//testcolor = vec4(float(gaussIndex%3==0), float(gaussIndex%3==1), float(gaussIndex%3==2), 1.0);
	testcolor = vec4(position, 1.0);

	flb.fragmentList[i].position = vec4(position, 1.0);
	//flb.fragmentList[i].position = vec4(depthValue / 60);
	//flb.fragmentList[i].position = testcolor;
}