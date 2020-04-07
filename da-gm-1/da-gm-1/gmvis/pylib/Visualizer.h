#pragma once
#include "pyimport.h"
#include <QApplication>
#include <thread>
#include <mutex>
#include <queue>
#include <memory>
#include "gmvis/core/GMRenderModes.h"
#include "gmvis/pylib/OffscreenRenderSurface.h"
#include "gmvis/core/PointCloud.h"
#include "gmvis/core/GaussianMixture.h"
#include "gmvis/core/DataLoader.h"

namespace gmvis::pylib {

	class Visualizer {
	public:
		static std::shared_ptr<Visualizer> create(bool async, int width, int height);

		Visualizer(bool async, int width, int height);
		Visualizer(Visualizer& v);
		void set_image_size(int width, int height);
		void set_view_matrix(py::array_t<float> viewmat);
		void set_camera_auto(bool camauto);
		void set_ellipsoid_rendering(bool ellipsoids = true, bool pointcloud = true);
		void set_ellipsoid_coloring(core::GMIsoellipsoidRenderMode colorMode, gmvis::core::GMIsoellipsoidColorRangeMode rangeMode, float min, float max);
		void set_density_rendering(bool density, gmvis::core::GMDensityRenderMode renderMode);
		void set_density_coloring(bool automatic, float autoperc, float min, float max);
		void set_density_accthreshold(bool automatic, float threshold);
		void set_pointclouds(py::array_t<float> pointclouds);
		void set_pointclouds_from_paths(py::list paths);
		void set_gaussian_mixtures(py::array_t<float> mixtures, bool isgmm);
		void set_gaussian_mixtures_from_paths(py::list paths, bool isgmm);
		void set_callback(py::object callback);
		py::array_t<float> render(int epoch);
		void finish();
		void forceStop();

	private:
		std::unique_ptr<QApplication> m_application;
		std::unique_ptr<OffscreenRenderSurface> m_surface;
		std::vector<std::unique_ptr<gmvis::core::PointCloud>> m_pointclouds;
		std::vector<std::unique_ptr<gmvis::core::GaussianMixture>> m_mixtures;
		std::unique_ptr<py::object> m_callback;
		std::unique_ptr<std::thread> m_thread;
		bool m_async = true;
		bool m_threadrunning = false;
		std::queue<std::function<void()>> m_commandrequests;
		std::mutex m_commandrequests_mutex;
		bool m_cameraAuto = false;

		void pushCommand(std::function<void()> cmd);
		py::array_t<float> processRenderRequest(int epoch);
		void visThread();
		void calculateCameraPositionByPointcloud(int index);
		void cleanup();

	};

}
