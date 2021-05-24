#pragma once

#include <QOffscreenSurface>
#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLDebugLogger>
#include "gmvis/core/Camera.h"
#include "gmvis/core/ScreenFBO.h"
#include "gmvis/core/PointCloudRenderer.h"
#include "gmvis/core/GMIsoellipsoidRenderer.h"
#include "gmvis/core/GMDensityRenderer.h"
#include "gmvis/core/GMPositionsRenderer.h"
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
		void setMixture(core::GaussianMixture<DECIMAL_TYPE>* mixture);
		void setPointcloud(core::PointCloud* pointcloud);
		void setWhiteMode(bool white);
		std::vector<std::unique_ptr<Image>> render();

		int getWidth() const;
		int getHeight() const;

		core::Camera* getCamera();

		core::PointCloudRenderer* getPointCloudRenderer();
		core::GMIsoellipsoidRenderer* getGMIsoellipsoidRenderer();
		core::GMPositionsRenderer * getGMPositionsRenderer();
		core::GMDensityRenderer* getGMDensityRenderer();

		void setEllipsoidPointcloudDisplayEnabled(bool displayEllipsoids, bool displayPoints, bool graybackground);
		void setGMPositionsDisplayEnabled(bool enabled, bool displayPoints);
		void setDensityDisplayEnabled(bool enabled);

		bool isEllipsoidDisplayEnabled() const;
		bool isGMPositionsDisplayEnabled() const;
		bool isDensityDisplayEnabled() const;

	public slots:
		void cleanup();

	protected slots:
		void messageLogged(const QOpenGLDebugMessage& msg);

	private:
		//Debug Logger
		std::unique_ptr<QOpenGLContext> m_context;
		std::unique_ptr<QOpenGLDebugLogger> m_debugLogger;

		//Settings
		bool m_sDisplayEllipsoids = false;
		bool m_sDisplayEllipsoids_Points = false;
		bool m_sDisplayEllipsoids_Gray = true;
		bool m_sDisplayGMPositions = false;
		bool m_sDisplayGMPositions_Points = true;
		bool m_sDisplayDensity = false;
		bool m_whitemode = false;

		//Renderers
		std::unique_ptr<core::PointCloudRenderer> m_pointcloudRenderer;
		std::unique_ptr<core::GMIsoellipsoidRenderer> m_isoellipsoidRenderer;
		std::unique_ptr<core::GMPositionsRenderer> m_positionRenderer;
		std::unique_ptr<core::GMDensityRenderer> m_densityRenderer;

		std::unique_ptr<core::ScreenFBO> m_fbo;

		//Matrices (TODO)
		std::unique_ptr<core::Camera> m_camera;

		void storeImage(std::string filename);
	};
}