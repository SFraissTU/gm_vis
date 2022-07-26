#pragma once
#include <qmatrix4x4.h>
#include <qvector3d.h>

namespace gmvis::core {

	class Camera {
	public:
		Camera();
		Camera(float fov, float aspectRatio, float nearP, float farP);

		float getFoV() const;
		float getAspectRatio() const;
		float getNear() const;
		float getFar() const;
		void setFoV(float fov);
		void setAspectRatio(float aspectRatio);
		void setNear(float nearP);
		void setFar(float farP);
		void rotateX(float amount);
		void rotateY(float amount);
		void zoom(float amount);
		void translateAlongScreen(const float x, const float y);
		//To set the view matrix directly. radius, angle, translation etc are not adapted
		void setViewMatrix(const QMatrix4x4& matrix);

		void setTranslation(QVector3D translation);
		void setXRotation(float xRot);
		void setYRotation(float yRot);
		void setRadius(float radius);
		void setTranslationSpeed(float speed);
		void setTranslationSpeedByBoundingBox(const QVector3D& min, const QVector3D& max);

		void setPositionByBoundingBox(const QVector3D& min, const QVector3D& max);

		const QMatrix4x4& getViewMatrix();
		const QMatrix4x4& getProjMatrix() const;
		const QVector3D& getPosition();
		const QVector3D getViewDirection();
		const QVector3D& getLookAt() const;

	private:
		//Camera settings
		float m_fov;
		float m_aspectRatio;
		float m_near;
		float m_far;

		const QVector3D m_absUp = QVector3D(0, 1, 0);
		QVector3D m_position = QVector3D(0, 0, 0);
		QVector3D m_up = QVector3D(0, 1, 0);
		QVector3D m_right = QVector3D(1, 0, 0);

		//Matrices
		QMatrix4x4 m_proj;
		QMatrix4x4 m_view;
		bool m_viewDirty = true;

		//Control stuff (From these the view matrix is calculated!)
		float m_xRot = 90.0f;
		float m_yRot = 0;
		float m_radius = 40.0f;
		float m_transSpeed = 0.005f;
		QVector3D m_translation = QVector3D(0, 0, 0);

		void recalculateProjectionMatrix();
		static float normalizeAngle(float angle);
	};
}