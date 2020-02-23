#pragma once

#include "GMDensityRasterizeRenderer.h"
#include "GMDensityRaycastRenderer.h"
#include "GMRenderModes.h"

class GMDensityRenderer {
public:
	GMDensityRenderer(QOpenGLFunctions_4_5_Core* gl, DisplaySettings* settings, Camera* camera, int width, int height, GMDensityRenderMode mode);
	void setMixture(GaussianMixture* mixture);
	void setSize(int width, int height);
	void render(GLuint preTexture);
	void setRenderMode(GMDensityRenderMode mode);
	void updateAccelerationData();
	void cleanup();

	static bool isAccelerated(GMDensityRenderMode mode);

private:
	GMDensityRenderMode m_renderMode;
	GMDensityRasterizeRenderer m_rasterizeRenderer;
	GMDensityRaycastRenderer m_raycastRenderer;
};