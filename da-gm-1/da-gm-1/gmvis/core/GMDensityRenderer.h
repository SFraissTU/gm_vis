#pragma once

#include "GMDensityRasterizeRenderer.h"
#include "GMDensityRaycastRenderer.h"
#include "GMRenderModes.h"

namespace gmvis::core {

	class GMDensityRenderer {
	public:
		GMDensityRenderer(QOpenGLFunctions_4_5_Core* gl, Camera* camera, int width, int height);
		void initialize();
		void setMixture(GaussianMixture<DECIMAL_TYPE>* mixture);
		bool hasMixture() const;
		void setSize(int width, int height);
		void render(GLuint preTexture, bool blend);
		void updateAccelerationData();
		void cleanup();

		const GMDensityRenderMode& getRenderMode() const;
		const double& getDensityMin() const;
		const double& getDensityMax() const;
		const bool& getDensityAuto() const;
		const double& getDensityAutoPercentage() const;
		const double& getAccelerationThreshold() const;
		const bool& getAccelerationThresholdAuto() const;
		const bool& getDensityCutoff() const;
		const bool& getLogarithmic() const;
		const double& getSuggestedDensityMaxLimit() const;
		const double& getSuggestedDensityLogMinLimit() const;
		const double& getSuggestedDensityLogMaxLimit() const;

		void setRenderMode(GMDensityRenderMode mode);
		void setDensityMin(double densityMin);
		void setDensityMax(double densityMax);
		void setDensityMinLog(double densityMinLog);
		void setDensityMaxLog(double densityMaxLog);
		void setDensityAuto(bool densityAuto);
		void setDensityAutoPercentage(double percentage);
		void setAccelerationThreshold(double accThreshold);
		void setAccelerationThresholdAuto(bool accThreshAuto);
		void setDensityCutoff(bool cutoff);
		void setLogarithmic(bool log);


		static bool isAccelerated(GMDensityRenderMode mode);

	private:
		QOpenGLFunctions_4_5_Core* m_gl;
		GaussianMixture<DECIMAL_TYPE>* m_mixture = nullptr;

		GMDensityRasterizeRenderer m_rasterizeRenderer;
		GMDensityRaycastRenderer m_raycastRenderer;

		//Settings
		GMDensityRenderMode m_sRenderMode = GMDensityRenderMode::ADDITIVE_ACC_PROJECTED;
		double m_sDensityMin = 0.0f;
		double m_sDensityMax = 0.0005f;
		double m_sDensityMinLog = -5.0f;
		double m_sDensityMaxLog = 5.0f;
		bool   m_sDensityAuto = false;
		double m_sDensityAutoPerc = 0.9;
		double m_sAccThreshold = 0.0000005;
		double m_sAccThresholdLog = -1000;
		bool   m_sAccThreshAuto = true;
		bool   m_sDensityCutoff = false;
		bool   m_sLogarithmic = false;
		double m_sDensitySuggestedMax = 1.0;
		double m_sDensitySuggestedLogMin = -4000;
		double m_sDensitySuggestedLogMax = 0;

		ScreenFBO m_fbo_intermediate;
		ScreenFBO m_fbo_final;

		GLuint m_col_bindingOutimg;
		GLuint m_col_bindingSumimg;
		GLuint m_col_bindingPreimg;
		GLuint m_col_locWidth;
		GLuint m_col_locHeight;
		GLuint m_col_locBlend;
		GLuint m_col_locTransferTex;
		GLuint m_col_locDensityMin;
		GLuint m_col_locDensityMax;
		GLuint m_col_locCutoff;
		GLuint m_col_locLogarithmic;

		GLuint m_texTransfer;

		std::unique_ptr<QOpenGLShaderProgram> m_program_coloring;

	};
}