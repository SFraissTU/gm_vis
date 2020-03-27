#pragma once
#include <memory>
#include <QApplication>
#include "gmvis/pylib/OffscreenRenderSurface.h"
#include "gmvis/core/PointCloud.h"
#include "gmvis/core/GaussianMixture.h"

namespace gmvis::pylib {

	class PythonInterface {
	public:
		static void startVisualizer();
		static void initialize(int width, int height);
		static void loadMixture(std::string path);
		static void render();
		static void exit();

	private:
		static std::unique_ptr<QApplication> application;
		static std::unique_ptr<OffscreenRenderSurface> visualizer;
		static std::unique_ptr<gmvis::core::PointCloud> pointcloud;
		static std::unique_ptr<gmvis::core::GaussianMixture> mixture;
	};
}