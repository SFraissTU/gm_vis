#pragma once

//#pragma push_macro("slots")
//#undef slots
#include <pybind11/pybind11.h>
//#pragma pop_macro("slots")

#include <memory>
#include <queue>
#include <mutex>
#include <QApplication>
#include "gmvis/pylib/OffscreenRenderSurface.h"
#include "gmvis/core/PointCloud.h"
#include "gmvis/core/GaussianMixture.h"
#include "gmvis/core/GMRenderModes.h"

namespace py = pybind11;

void pyprint(std::string str) {
#ifdef PY_LIB
	py::print(str);
#else
	qDebug() << QString(str.c_str()) << "\n";
#endif
}

namespace gmvis::pylib {

	class PythonInterface {

	public:
		static void initialize(int width, int height);
		static void set_image_size(int width, int height);
		static void set_camera_auto(bool mode);
		static void set_camera_position(float lookat_x, float lookat_y, float lookat_z, float xRot, float yRot, float radius);
		static void set_ellipsoid_rendering(bool ellipsoids, bool pointcloud);
		static void set_ellipsoid_coloring(gmvis::core::GMIsoellipsoidRenderMode colorMode, gmvis::core::GMIsoellipsoidColorRangeMode rangeMode, float min, float max);
		static void set_density_rendering(bool density, gmvis::core::GMDensityRenderMode renderMode);
		static void set_density_coloring(bool automatic, float autoperc, float min, float max);
		static void set_density_accthreshold(bool automatic, float threshold);
		static void set_pointcloud(std::string path);
		static void set_tensorboard_writer(py::object writer);
		static void render(std::string gmpath, int epoch);
		static void finish();
		static void shutdown();

		static py::array_t<float> buffertest();

	private:
		static PythonInterface pinterface;

		std::unique_ptr<QApplication> application;
		std::unique_ptr<OffscreenRenderSurface> visualizer;
		std::unique_ptr<gmvis::core::PointCloud> pointcloud;
		std::unique_ptr<gmvis::core::GaussianMixture> mixture;
		std::unique_ptr<py::object> writer;
		std::unique_ptr<std::thread> thread;
		bool threadrunning = false;
		std::queue<std::function<void()>> commandrequests;
		std::mutex commandrequests_mutex;
		std::queue<std::pair<std::string, int>> renderrequests;
		std::mutex renderrequests_mutex;

		bool cameraAuto = false;

		static void pushCommand(std::function<void()> cmd);
		static void visThread();
		static void calculateCameraPositionByPointcloud();
		//static void calculateCameraPositionByGM(); (Nice to Have)
	};
}