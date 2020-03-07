#pragma once

enum class GMDensityRenderMode {
	ADDITIVE = 1,
	ADDITIVE_ACC_OCTREE = 2,
	ADDITIVE_ACC_PROJECTED = 3,
	ADDITIVE_SAMPLING_OCTREE = 4
};

enum class GMIsoellipsoidRenderMode {
	COLOR_UNIFORM = 1,
	COLOR_WEIGHT = 2,
	COLOR_AMPLITUDE = 3
};

enum class GMIsoellipsoidColorRangeMode {
	RANGE_MANUAL = 1,
	RANGE_MINMAX = 2,
	RANGE_MEDMED = 3
};