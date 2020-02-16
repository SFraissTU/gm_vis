#include "GMDensityRenderer.h"

GMDensityRenderer::GMDensityRenderer(QOpenGLFunctions_4_5_Core* gl, DisplaySettings* settings, Camera* camera, int width, int height, GMDensityRenderMode mode)
	: m_renderMode(mode),
	m_rasterizeRenderer(GMDensityRasterizeRenderer(gl, settings, camera, width, height)),
	m_raycastRenderer(GMDensityRaycastRenderer(gl, settings, camera, width, height))
{
	
}

void GMDensityRenderer::setMixture(GaussianMixture* mixture)
{
	m_rasterizeRenderer.setMixture(mixture);
	m_raycastRenderer.setMixture(mixture);
}

void GMDensityRenderer::setSize(int width, int height)
{
	m_rasterizeRenderer.setSize(width, height);
	m_raycastRenderer.setSize(width, height);
}

void GMDensityRenderer::render(GLuint preTexture)
{
	if (m_renderMode == GMDensityRenderMode::ADDITIVE_ACC_PROJECTED) {
		m_rasterizeRenderer.render(preTexture);
	}
	else {
		m_raycastRenderer.render(preTexture);
	}
}

void GMDensityRenderer::setRenderMode(GMDensityRenderMode mode)
{
	m_renderMode = mode;
	switch (m_renderMode) {
	case GMDensityRenderMode::ADDITIVE:
		m_raycastRenderer.disableAccelerationStructure();
		break;
	case GMDensityRenderMode::ADDITIVE_ACC_OCTREE:
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
	m_rasterizeRenderer.updateAccelerationData();
	m_raycastRenderer.rebuildAccelerationStructure();
}

void GMDensityRenderer::cleanup()
{
	m_rasterizeRenderer.cleanup();
	m_raycastRenderer.cleanup();
}

bool GMDensityRenderer::isAccelerated(GMDensityRenderMode mode)
{
	return mode == GMDensityRenderMode::ADDITIVE_ACC_OCTREE || mode == GMDensityRenderMode::ADDITIVE_ACC_PROJECTED || mode == GMDensityRenderMode::ALPHA_ACC_OCTREE;
}
