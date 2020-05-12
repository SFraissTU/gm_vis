#include "Camera.h"
#include <qmath.h>

using namespace gmvis::core;

Camera::Camera()
	: m_fov(60.0f), m_aspectRatio(1), m_near(0.1), m_far(1000)
{
	recalculateProjectionMatrix();
}

Camera::Camera(float fov, float aspectRatio, float nearP, float farP)
	: m_fov(fov), m_aspectRatio(aspectRatio), m_near(nearP), m_far(farP)
{
	recalculateProjectionMatrix();
}

float Camera::getFoV() const
{
	return m_fov;
}

float Camera::getAspectRatio() const
{
	return m_aspectRatio;
}

float Camera::getNear() const
{
	return m_near;
}

float Camera::getFar() const
{
	return m_far;
}

void Camera::setFoV(float fov)
{
	m_fov = fov;
	recalculateProjectionMatrix();
}

void Camera::setAspectRatio(float aspectRatio)
{
	m_aspectRatio = aspectRatio;
	recalculateProjectionMatrix();
}

void Camera::setNear(float nearP)
{
	m_near = nearP;
	recalculateProjectionMatrix();
}

void Camera::setFar(float farP)
{
	m_far = farP;
	recalculateProjectionMatrix();
}

void Camera::rotateX(float amount)
{
	m_xRot = m_xRot + amount;
	if (m_xRot > 179.5f) {
		m_xRot = 179.5f;
	}
	if (m_xRot < 0.5f) {
		m_xRot = 0.5f;
	}
	m_xRot = normalizeAngle(m_xRot);
	m_viewDirty = true;
}

void Camera::rotateY(float amount)
{
	m_yRot = normalizeAngle(m_yRot + amount);
	m_viewDirty = true;
}

void Camera::zoom(float amount)
{
	m_radius = std::max(0.5f, m_radius - amount);
	m_viewDirty = true;
}

void Camera::translateAlongScreen(const float x, const float y)
{
	m_translation += x * m_right + y * m_up;
	m_viewDirty = true;
}

void gmvis::core::Camera::setViewMatrix(const QMatrix4x4& matrix)
{
	m_view = matrix;
	m_viewDirty = false;
}

void gmvis::core::Camera::setTranslation(QVector3D translation)
{
	m_translation = translation;
	m_viewDirty = true;
}

void gmvis::core::Camera::setXRotation(float xRot)
{
	m_xRot = xRot;
	m_viewDirty = true;
}

void gmvis::core::Camera::setYRotation(float yRot)
{
	m_yRot = yRot;
	m_viewDirty = true;
}

void gmvis::core::Camera::setRadius(float radius)
{
	m_radius = radius;
	m_viewDirty = true;
}

void gmvis::core::Camera::setPositionByBoundingBox(QVector3D min, QVector3D max)
{
	QVector3D center = (min + max) / 2.0;
	QVector3D extend = (max - min) / 2.0;
	setTranslation(center);
	setRadius(std::max(extend.x(), std::max(extend.y(), extend.z())) * 3);
	setXRotation(135);
	setYRotation(45);
}

const QMatrix4x4& Camera::getViewMatrix()
{
	if (m_viewDirty) {
		m_view.setToIdentity();
		
		float xangle = qDegreesToRadians(180.0f - m_xRot);
		float yangle = qDegreesToRadians(m_yRot);
		float z = m_radius * sin(xangle) * cos(yangle);
		float x = m_radius * sin(xangle) * sin(yangle);
		float y = m_radius * cos(xangle);
		m_position = QVector3D(x, y, z) + m_translation;
		auto forward = (m_translation - m_position).normalized();
		m_right = QVector3D::crossProduct(m_absUp, forward).normalized();
		m_up = QVector3D::crossProduct(forward, m_right).normalized();
		m_view.lookAt(m_position, m_translation, m_absUp);
		qDebug() << m_position << "\n";

		//m_view.translate(0, 0, -m_radius);
		//m_view.rotate(180.0f - m_xRot, 1, 0, 0);
		//m_view.rotate(m_yRot, 0, 1, 0);
		//m_view.rotate(m_zRot, 0, 0, 1);
		//TODO: Translation
		m_viewDirty = false;
	}
	return m_view;
}

const QMatrix4x4& Camera::getProjMatrix() const
{
	return m_proj;
}

const QVector3D& gmvis::core::Camera::getPosition() const
{
	return m_position;
}

void Camera::recalculateProjectionMatrix()
{
	m_proj.setToIdentity();
	m_proj.perspective(m_fov, m_aspectRatio, m_near, m_far);
}

float Camera::normalizeAngle(float angle)
{
	while (angle < 0)
		angle += 360.f;
	while (angle > 360.f)
		angle -= 360.f;
	return angle;
}