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
		void set_camera_lookat(std::tuple<float, float, float> position, std::tuple<float, float, float> lookAt, std::tuple<float, float, float> up);
		void set_camera_auto(bool camauto);
		void set_whitemode(bool white);
		void set_ellipsoids_pc_rendering(bool ellipsoids, bool pointcloud=false, bool gray=true);
		void set_ellipsoids_colormode(core::GMColoringRenderMode colorMode);
		void set_ellipsoids_rangemode(gmvis::core::GMColorRangeMode rangeMode, float min=1.0, float max=0.0);
		void set_positions_rendering(bool positions, bool pointcloud=true);
		void set_positions_colormode(core::GMColoringRenderMode colorMode);
		void set_positions_rangemode(gmvis::core::GMColorRangeMode rangeMode, float min = 1.0, float max = 0.0);
		void set_density_rendering(bool density);
		void set_density_rendermode(gmvis::core::GMDensityRenderMode renderMode);
		void set_density_range_auto(float autoperc=0.75);
		void set_density_range_manual(float min, float max);
		void set_density_logarithmic(bool logarithmic);
		void set_density_accthreshold(bool automatic, float threshold);
		void set_pointclouds(py::array_t<float> pointclouds);
		void set_pointclouds_from_paths(py::list paths);
		void set_gaussian_mixtures(py::array_t<DECIMAL_TYPE> mixtures, bool isgmm);
		void set_gaussian_mixtures_from_paths(py::list paths, bool isgmm);
		void set_callback(py::object callback);
		py::array_t<float> render(int epoch);
		void finish();
		void forceStop();

	private:
		std::unique_ptr<QApplication> m_application;
		std::unique_ptr<OffscreenRenderSurface> m_surface;
		std::vector<std::unique_ptr<gmvis::core::PointCloud>> m_pointclouds;
		std::vector<std::unique_ptr<gmvis::core::GaussianMixture<DECIMAL_TYPE>>> m_mixtures;
		std::unique_ptr<py::object> m_callback;
		std::unique_ptr<std::thread> m_thread;
		bool m_async = true;
		bool m_threadrunning = false;
		std::queue<std::function<void()>> m_commandrequests;
		std::mutex m_commandrequests_mutex;
		bool m_cameraAuto = false;
		bool m_densityAuto = false;
		float m_densityAutoPerc = 0.75;

		void pushCommand(std::function<void()> cmd);
		py::array_t<float> processRenderRequest(int epoch);
		void visThread();
		void calculateAutoCameraPosition(int index);
		void cleanup();

	};

}
