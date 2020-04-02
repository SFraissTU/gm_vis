#pragma once

#include "pyimport.h"

#include <memory>
#include <queue>
#include <mutex>
#include <QApplication>
#include "gmvis/pylib/OffscreenRenderSurface.h"
#include "gmvis/core/PointCloud.h"
#include "gmvis/core/GaussianMixture.h"
#include "gmvis/core/GMRenderModes.h"
#include "gmvis/pylib/Visualizer.h"

namespace py = pybind11;

namespace gmvis::pylib {

	class PythonInterface {

	public:
		static void initialize(bool async, int width, int height);
		static void set_image_size(int width, int height);
		static void set_camera_auto(bool mode);
		static void set_camera_position(float lookat_x, float lookat_y, float lookat_z, float xRot, float yRot, float radius);
		static void set_ellipsoid_rendering(bool ellipsoids, bool pointcloud);
		static void set_ellipsoid_coloring(gmvis::core::GMIsoellipsoidRenderMode colorMode, gmvis::core::GMIsoellipsoidColorRangeMode rangeMode, float min, float max);
		static void set_density_rendering(bool density, gmvis::core::GMDensityRenderMode renderMode);
		static void set_density_coloring(bool automatic, float autoperc, float min, float max);
		static void set_density_accthreshold(bool automatic, float threshold);
		static void set_pointcloud(std::string path);
		static void set_callback(py::object callback);
		static void render(std::string gmpath, int epoch);
		static void finish();
		static void forceStop();

		static py::array_t<float> buffertest();

	private:
		static PythonInterface pinterface;

		std::unique_ptr<QApplication> application;
		std::unique_ptr<OffscreenRenderSurface> visualizer;
		std::unique_ptr<gmvis::core::PointCloud> pointcloud;
		std::unique_ptr<gmvis::core::GaussianMixture> mixture;
		std::unique_ptr<py::object> callback;
		std::unique_ptr<std::thread> thread;
		bool async = true;
		bool threadrunning = false;
		std::queue<std::function<void()>> commandrequests;
		std::mutex commandrequests_mutex;

		bool cameraAuto = false;

		static void pushCommand(std::function<void()> cmd);
		static void processRenderRequest(std::string gmpath, int epoch);
		static void visThread();
		static void calculateCameraPositionByPointcloud();
		static void cleanup();
		//static void calculateCameraPositionByGM(); (Nice to Have)
	};
}