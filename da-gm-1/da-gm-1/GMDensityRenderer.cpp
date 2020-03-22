#include "GMDensityRenderer.h"

GMDensityRenderer::GMDensityRenderer(QOpenGLFunctions_4_5_Core* gl, Camera* camera, int width, int height) : 
	m_rasterizeRenderer(GMDensityRasterizeRenderer(gl, camera, width, height)),
	m_raycastRenderer(GMDensityRaycastRenderer(gl, camera, width, height))
{
	m_raycastRenderer.setAccelerationThreshold(m_sAccThreshold);
}

void GMDensityRenderer::initialize()
{
	m_rasterizeRenderer.initialize();
	m_raycastRenderer.initialize();
	setRenderMode(m_sRenderMode);
}

void GMDensityRenderer::setMixture(GaussianMixture* mixture)
{
	m_rasterizeRenderer.setMixture(mixture, m_sAccThreshold);
	m_raycastRenderer.setMixture(mixture);
}

void GMDensityRenderer::setSize(int width, int height)
{
	m_rasterizeRenderer.setSize(width, height);
	m_raycastRenderer.setSize(width, height);
}

void GMDensityRenderer::render(GLuint preTexture, bool blend)
{
	if (m_sRenderMode == GMDensityRenderMode::ADDITIVE_ACC_PROJECTED) {
		m_rasterizeRenderer.render(preTexture, blend, m_sDensityMin, m_sDensityMax, m_sDensityAuto, m_sDensityAutoPerc);
	}
	else {
		m_raycastRenderer.render(preTexture, blend, m_sDensityMin, m_sDensityMax, m_sDensityAuto, m_sDensityAutoPerc);
	}
}

void GMDensityRenderer::setRenderMode(GMDensityRenderMode mode)
{
	m_sRenderMode = mode;
	switch (m_sRenderMode) {
	case GMDensityRenderMode::ADDITIVE:
		m_raycastRenderer.disableAccelerationStructure();
		break;
	case GMDensityRenderMode::ADDITIVE_SAMPLING_OCTREE:
		m_raycastRenderer.setSampling(true);
		m_raycastRenderer.enableAccelerationStructure();
		break;
	case GMDensityRenderMode::ADDITIVE_ACC_OCTREE:
		m_raycastRenderer.setSampling(false);
		m_raycastRenderer.enableAccelerationStructure();
		break;
	case GMDensityRenderMode::ADDITIVE_ACC_PROJECTED:
		break;
	default:
		qDebug() << "Unsupported Render Mode\n";
	}
}

void GMDensityRenderer::updateAccelerationData()
{
	m_rasterizeRenderer.updateAccelerationData(m_sAccThreshold);
	m_raycastRenderer.rebuildAccelerationStructure();
}

void GMDensityRenderer::cleanup()
{
	m_rasterizeRenderer.cleanup();
	m_raycastRenderer.cleanup();
}

void GMDensityRenderer::setDensityMin(double densityMin)
{
	m_sDensityMin = densityMin;
}

void GMDensityRenderer::setDensityMax(double densityMax)
{
	m_sDensityMax = densityMax;
	if (m_sAccThreshAuto) {
		m_sAccThreshold = std::max(m_sDensityMax * 0.0001, 0.000000000001);
	}
}

void GMDensityRenderer::setDensityAuto(bool densityAuto)
{
	m_sDensityAuto = densityAuto;
}

void GMDensityRenderer::setDensityAutoPercentage(double percentage)
{
	m_sDensityAutoPerc = percentage;
}

void GMDensityRenderer::setAccelerationThreshold(double accThreshold)
{
	m_sAccThreshold = accThreshold;
	m_raycastRenderer.setAccelerationThreshold(accThreshold);
}

void GMDensityRenderer::setAccelerationThresholdAuto(bool accThreshAuto)
{
	m_sAccThreshAuto = accThreshAuto;
	if (m_sAccThreshAuto) {
		m_sAccThreshold = m_sDensityMax * 0.0001;
	}
}

const GMDensityRenderMode& GMDensityRenderer::getRenderMode() const
{
	return m_sRenderMode;
}

const double& GMDensityRenderer::getDensityMin() const
{
	return m_sDensityMin;
}

const double& GMDensityRenderer::getDensityMax() const
{
	return m_sDensityMax;
}

const bool& GMDensityRenderer::getDensityAuto() const
{
	return m_sDensityAuto;
}

const double& GMDensityRenderer::getDensityAutoPercentage() const
{
	return m_sDensityAutoPerc;
}

const double& GMDensityRenderer::getAccelerationThreshold() const
{
	return m_sAccThreshold;
}

const bool& GMDensityRenderer::getAccelerationThresholdAuto() const
{
	return m_sAccThreshAuto;
}

bool GMDensityRenderer::isAccelerated(GMDensityRenderMode mode)
{
	return mode == GMDensityRenderMode::ADDITIVE_ACC_OCTREE || mode == GMDensityRenderMode::ADDITIVE_ACC_PROJECTED || mode == GMDensityRenderMode::ADDITIVE_SAMPLING_OCTREE;
}
