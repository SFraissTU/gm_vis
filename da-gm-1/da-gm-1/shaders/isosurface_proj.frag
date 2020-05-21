#version 450

// Isosurface Visualization
// Fragment Shader. Calcualtes contribution to pixel sum

flat in int gaussIndex;

out float frag_startidx;

void main() {
	frag_startidx = 1;
}