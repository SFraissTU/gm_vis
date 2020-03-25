#pragma once
#include <memory>
#include "gmvis/pylib/OffscreenRenderSurface.h"

namespace gmvis::pylib {

	class PythonInterface {
	public:
		static void startVisualizer();
		static void initialize();

	private:
		static std::unique_ptr<OffscreenRenderSurface> visualizer;
	};
}