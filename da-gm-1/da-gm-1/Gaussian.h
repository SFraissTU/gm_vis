#pragma once

struct Gaussian {
	float x;
	float y;
	float z;
	float covxx;
	float covxy;
	float covxz;
	float covyy;
	float covyz;
	float covzz;
	float weight;
};