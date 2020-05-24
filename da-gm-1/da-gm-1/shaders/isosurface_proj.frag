#version 450

// Isosurface Visualization
// Fragment Shader. Calcualtes contribution to pixel sum

flat in int gaussIndex;
in float depthValue;

struct FragmentData {
	float z;
	int gIndex;
	int flags;
	int next;
};

layout(std430, binding=0) buffer FragmentList {
	FragmentData[] fragmentList;
} flb;

layout(binding=1) uniform atomic_uint listSize;

layout(r32i, binding=2) uniform iimage2D img_startidx;

void main() {
	uint i = atomicCounterIncrement(listSize);
	if (i >= flb.fragmentList.length()) {
		atomicCounterDecrement(listSize);
		discard;
	}
	flb.fragmentList[i].z = depthValue;
	flb.fragmentList[i].gIndex = gaussIndex;
	flb.fragmentList[i].flags = int(gl_FrontFacing);
	int oldhead = imageAtomicExchange(img_startidx, ivec2(gl_FragCoord.xy * imageSize(img_startidx)), int(i));
	flb.fragmentList[i].next = oldhead;
}