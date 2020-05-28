#version 450

// Isosurface Visualization
// Fragment Shader. Calcualtes contribution to pixel sum

flat in int gaussIndex;
in float depthValue;

out vec4 testcolor;

struct FragmentData {
	float z;
	int gIndex;
	int flags;
	int next;
};

layout(std430, binding=0) buffer FragmentList {
	FragmentData[] fragmentList;
} flb;

layout(binding=1, offset=0) uniform atomic_uint listSize;

layout(r32i) uniform iimage2D img_startidx;

uniform uint maxFragmentListLength;

void main() {
	uint i = atomicCounterIncrement(listSize);
	//TODO: For some reason Fragment List is empty!
	if (i >= maxFragmentListLength) {
		atomicCounterDecrement(listSize);
		//testcolor = vec4(1.0, 0.0, 0.0, 1.0);
		discard;
		return;
	}
	flb.fragmentList[i].z = -depthValue;
	flb.fragmentList[i].gIndex = gaussIndex;
	flb.fragmentList[i].flags = int(gl_FrontFacing);
	int oldhead = imageAtomicExchange(img_startidx, ivec2(gl_FragCoord.xy), int(i));
	//imageStore(img_startidx, ivec2(gl_FragCoord.xy * imageSize(img_startidx)), ivec4(1));
	//imageSize is wrong anyway, as this might be larger than w + h
	//imageStore(img_startidx, ivec2(gl_FragCoord.xy), ivec4(1));
	flb.fragmentList[i].next = oldhead;
	testcolor = vec4(1.0);
}