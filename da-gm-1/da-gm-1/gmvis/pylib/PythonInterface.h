#pragma once
#include <memory>
#include <QApplication>
#include "gmvis/pylib/OffscreenRenderSurface.h"

namespace gmvis::pylib {

	class PythonInterface {
	public:
		static void startVisualizer();
		static void initialize();
		static void exit();

	private:
		static std::unique_ptr<QApplication> application;
		static std::unique_ptr<OffscreenRenderSurface> visualizer;
	};
}