#pragma once

enum class GMDensityRenderMode {
	ADDITIVE = 1,
	ADDITIVE_ACC_OCTREE = 2,
	ADDITIVE_ACC_PROJECTED = 3,
	ALPHA = 4,
	ALPHA_ACC_OCTREE = 5
};

enum class GMIsoellipsoidRenderMode {
	COLOR_UNIFORM = 1,
	COLOR_WEIGHT = 2,
	COLOR_AMPLITUDE = 3
};