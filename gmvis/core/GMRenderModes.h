#pragma once

namespace gmvis::core {

	enum class GMDensityRenderMode {
		ADDITIVE_EXACT = 1,
		ADDITIVE_ACC_OCTREE = 2,
		ADDITIVE_ACC_PROJECTED = 3,
		ADDITIVE_SAMPLING_OCTREE = 4
	};

	enum class GMColoringRenderMode {
		COLOR_UNIFORM = 1,
		COLOR_WEIGHT = 2,
		COLOR_AMPLITUDE = 3
	};

	enum class GMColorRangeMode {
		RANGE_MANUAL = 1,
		RANGE_MINMAX = 2,
        RANGE_MEDMED = 3,
        RANGE_MAXABSMINMAX = 4
	};
}
