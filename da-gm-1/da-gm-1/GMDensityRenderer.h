#pragma once

#include "GMDensityRasterizeRenderer.h"
#include "GMDensityRaycastRenderer.h"
#include "GMRenderModes.h"

class GMDensityRenderer {
public:
	GMDensityRenderer(QOpenGLFunctions_4_5_Core* gl, Camera* camera, int width, int height);
	void initialize();
	void setMixture(GaussianMixture* mixture);
	void setSize(int width, int height);
	void render(GLuint preTexture, bool blend);
	void updateAccelerationData();
	void cleanup();

	const GMDensityRenderMode& getRenderMode() const;
	const double& getDensityMin() const;
	const double& getDensityMax() const;
	const double& getAccelerationThreshold() const;
	const bool& getAccelerationThresholdAuto() const;

	void setRenderMode(GMDensityRenderMode mode);
	void setDensityMin(double densityMin);
	void setDensityMax(double densityMax);
	void setAccelerationThreshold(double accThreshold);
	void setAccelerationThresholdAuto(bool accThreshAuto);


	static bool isAccelerated(GMDensityRenderMode mode);

private:
	GMDensityRasterizeRenderer m_rasterizeRenderer;
	GMDensityRaycastRenderer m_raycastRenderer;

	//Settings
	GMDensityRenderMode m_sRenderMode = GMDensityRenderMode::ADDITIVE_ACC_PROJECTED;
	double m_sDensityMin   = 0.0f;
	double m_sDensityMax   = 0.0005f;
	double m_sAccThreshold  = 0.0000005;
	bool   m_sAccThreshAuto = true;
	
};