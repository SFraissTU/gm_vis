#pragma once

#include <QOffscreenSurface>
#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLDebugLogger>
#include "gmvis/core/Camera.h"
#include "gmvis/core/ScreenFBO.h"
#include "gmvis/core/PointCloudRenderer.h"
#include "gmvis/core/GMIsoellipsoidRenderer.h"
#include "gmvis/core/GMDensityRenderer.h"
#include "gmvis/core/GaussianMixture.h"
#include "gmvis/pylib/Image.h"

namespace gmvis::pylib {

	class OffscreenRenderSurface : public QOffscreenSurface, public QOpenGLFunctions_4_5_Core {
		Q_OBJECT

	public:
		OffscreenRenderSurface();
		~OffscreenRenderSurface();

		void initialize(int width, int height);
		void setSize(int width, int height);
		void setMixture(core::GaussianMixture* mixture);
		void setPointcloud(core::PointCloud* pointcloud);
		std::vector<std::unique_ptr<Image>> render();

		int getWidth() const;
		int getHeight() const;

		core::Camera* getCamera();

		core::PointCloudRenderer* getPointCloudRenderer();
		core::GMIsoellipsoidRenderer* getGMIsoellipsoidRenderer();
		core::GMDensityRenderer* getGMDensityRenderer();

		void setPointDisplayEnabled(bool enabled);
		void setEllipsoidDisplayEnabled(bool enabled);
		void setDensityDisplayEnabled(bool enabled);

		bool isPointDisplayEnabled();
		bool isEllipsoidDisplayEnabled();
		bool isDensityDisplayEnabled();

	public slots:
		void cleanup();

	protected slots:
		void messageLogged(const QOpenGLDebugMessage& msg);

	private:
		//Debug Logger
		std::unique_ptr<QOpenGLContext> m_context;
		std::unique_ptr<QOpenGLDebugLogger> m_debugLogger;

		//Settings
		bool m_sDisplayPoints = true;
		bool m_sDisplayEllipsoids = true;
		bool m_sDisplayDensity = true;

		//Renderers
		std::unique_ptr<core::PointCloudRenderer> m_pointcloudRenderer;
		std::unique_ptr<core::GMIsoellipsoidRenderer> m_isoellipsoidRenderer;
		std::unique_ptr<core::GMDensityRenderer> m_densityRenderer;

		std::unique_ptr<core::ScreenFBO> m_fbo;

		//Matrices (TODO)
		std::unique_ptr<core::Camera> m_camera;

		void storeImage(std::string filename);
	};
}