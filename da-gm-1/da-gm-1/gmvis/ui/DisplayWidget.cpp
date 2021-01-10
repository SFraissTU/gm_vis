#include "DisplayWidget.h"

#include <memory>
#include <QTime>
#include <cmath>

using namespace gmvis::ui;
using namespace gmvis::core;

//Partialy used this: https://code.qt.io/cgit/qt/qtbase.git/tree/examples/opengl/hellogl2?h=5.13

DisplayWidget::DisplayWidget(QWidget* parent) : QOpenGLWidget(parent)
{
	QSurfaceFormat format;
	format.setMajorVersion(4);
	format.setMinorVersion(5);
	format.setProfile(QSurfaceFormat::CoreProfile);
	format.setOption(QSurfaceFormat::DebugContext);
	setFormat(format);
	setFocusPolicy(Qt::StrongFocus);

	m_camera = std::make_unique<Camera>(60.0f, GLfloat(width()) / height(), 0.01f, 1000.0f);
	auto gl = static_cast<QOpenGLFunctions_4_5_Core*>(this);
	m_pointcloudRenderer = std::make_unique<PointCloudRenderer>(gl, m_camera.get());
	m_isoellipsoidRenderer = std::make_unique<GMIsoellipsoidRenderer>(gl, m_camera.get());
	m_positionsRenderer = std::make_unique<GMPositionsRenderer>(gl, m_camera.get());
	m_densityRenderer = std::make_unique<GMDensityRenderer>(gl, m_camera.get(), width(), height());
	m_isosurfaceRenderer = std::make_unique<GMIsosurfaceRenderer>(gl, m_camera.get(), width(), height());
	m_lineRenderer = std::make_unique<LineRenderer>(static_cast<QOpenGLFunctions_4_5_Core*>(this), m_camera.get());
}

DisplayWidget::~DisplayWidget() {
	cleanup();
}

QSize DisplayWidget::minimumSizeHint() const {
	return QSize(50, 50);
}

QSize DisplayWidget::sizeHint() const {
	return QSize(500, 500);
}

void DisplayWidget::setPointDisplayEnabled(bool enabled)
{
	m_sDisplayPoints = enabled;
}

void DisplayWidget::setEllipsoidDisplayEnabled(bool enabled)
{
	m_sDisplayEllipsoids = enabled;
}

void gmvis::ui::DisplayWidget::setGMPositionsDisplayEnabled(bool enabled)
{
	m_sDisplayGMPositions = enabled;
}

void gmvis::ui::DisplayWidget::setIsosurfaceDisplayEnabled(bool enabled)
{
	m_sDisplayIsosurface = enabled;
}

void DisplayWidget::setDensityDisplayEnabled(bool enabled)
{
	m_sDisplayDensity = enabled;
}

bool DisplayWidget::isPointDisplayEnabled() const
{
	return m_sDisplayPoints;
}

bool DisplayWidget::isEllipsoidDisplayEnabled() const
{
	return m_sDisplayEllipsoids;
}

bool DisplayWidget::isDensityDisplayEnabled() const
{
	return m_sDisplayDensity;
}

bool gmvis::ui::DisplayWidget::isIsosurfaceDisplayEnabled() const
{
	return m_sDisplayIsosurface;
}

bool gmvis::ui::DisplayWidget::isGMVisibleInAnyWay() const
{
	return m_sDisplayEllipsoids || m_sDisplayGMPositions || m_sDisplayDensity;
}

bool gmvis::ui::DisplayWidget::isGMPositionsDisplayEnabled() const
{
	return m_sDisplayGMPositions;
}

PointCloudRenderer* DisplayWidget::getPointCloudRenderer()
{
	return m_pointcloudRenderer.get();
}

GMIsoellipsoidRenderer* DisplayWidget::getGMIsoellipsoidRenderer()
{
	return m_isoellipsoidRenderer.get();
}

GMPositionsRenderer* DisplayWidget::getGMPositionsRenderer()
{
	return m_positionsRenderer.get();
}

GMIsosurfaceRenderer* gmvis::ui::DisplayWidget::getGMIsosurfaceRenderer()
{
	return m_isosurfaceRenderer.get();
}

GMDensityRenderer* DisplayWidget::getGMDensityRenderer()
{
	return m_densityRenderer.get();
}

LineRenderer* DisplayWidget::getLineRenderer()
{
	return m_lineRenderer.get();
}

Camera* gmvis::ui::DisplayWidget::getCamera()
{
	return m_camera.get();
}

void gmvis::ui::DisplayWidget::setMixture(GaussianMixture<float>* mixture)
{
	m_isoellipsoidRenderer->setMixture(mixture);
	m_positionsRenderer->setMixture(mixture);
	m_isosurfaceRenderer->setMixture(mixture, 0.00001);
	m_densityRenderer->setMixture(mixture);
	m_mixture = mixture;
}

void DisplayWidget::cleanup() {
	makeCurrent();
	//This could be called several times, so make sure things are not deleted already
	if (m_pointcloudRenderer.get()) {
		m_pointcloudRenderer->cleanup();
		m_pointcloudRenderer.reset();
		m_lineRenderer->cleanup();
		m_lineRenderer.reset();
		m_isoellipsoidRenderer->cleanup();
		m_isoellipsoidRenderer.reset();
		m_positionsRenderer->cleanup();
		m_positionsRenderer.reset();
		m_densityRenderer->cleanup();
		m_densityRenderer.reset();
		m_debugLogger.reset();
		m_fboIntermediate->cleanup();
		m_fboIntermediate.reset();
	}
	doneCurrent();
}

void DisplayWidget::initializeGL() {
	(void)connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &DisplayWidget::cleanup);

	initializeOpenGLFunctions();

	m_pointcloudRenderer->initialize();
	m_isoellipsoidRenderer->initialize();
	m_positionsRenderer->initialize();
	m_densityRenderer->initialize();
	m_isosurfaceRenderer->initialize();
	m_lineRenderer->initialize();

	m_fboIntermediate = std::make_unique<ScreenFBO>(static_cast<QOpenGLFunctions_4_5_Core*>(this), width(), height());
	m_fboIntermediate->initialize();
	m_fboIntermediate->attachColorTexture();
	m_fboIntermediate->attachSinglevalueFloatTexture(1);
	m_fboIntermediate->attachDepthTexture();

#if _DEBUG
	m_debugLogger = std::make_unique<QOpenGLDebugLogger>(this);
	if (m_debugLogger->initialize()) {
		qDebug() << "GL_DEBUG Debug Logger " << m_debugLogger.get() << "\n";
		(void)connect(m_debugLogger.get(), &QOpenGLDebugLogger::messageLogged, this, &DisplayWidget::messageLogged);
		m_debugLogger->startLogging();
	}
#endif
	
	auto background = QColor(0, 0, 0);
	glClearColor(background.redF(), background.blueF(), background.greenF(), 1);
}

void DisplayWidget::paintGL()
{
	GLint defaultFbo = 0;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &defaultFbo);

	if (m_sDisplayIsosurface) {
		GLuint query;
		glGenQueries(1, &query);
		glBeginQuery(GL_TIME_ELAPSED, query);

		int width = m_fboIntermediate->getWidth();
		int height = m_fboIntermediate->getHeight();
		m_isosurfaceRenderer->render(width, height);

		glEndQuery(GL_TIME_ELAPSED);

		GLint done = 0;
		while (!done) {
			glGetQueryObjectiv(query, GL_QUERY_RESULT_AVAILABLE, &done);
		}

		GLuint64 elapsed;
		glGetQueryObjectui64v(query, GL_QUERY_RESULT, &elapsed);
		qDebug() << elapsed / 1000000.0 << "ms\n";
		return;
	}

	bool renderDensity = m_sDisplayDensity && m_densityRenderer->hasMixture();

	//Only render points and ellipsoids if blending mode requires it
	if (m_sDisplayPoints || m_sDisplayEllipsoids || m_sDisplayGMPositions) {
		//Only set FBO if we will render volume later on
		glBindFramebuffer(GL_FRAMEBUFFER, m_fboIntermediate->getID());

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glDisable(GL_BLEND);
		int width = m_fboIntermediate->getWidth();
		int height = m_fboIntermediate->getHeight();
		glViewport(0, 0, width, height);

		if (m_sDisplayGMPositions) {
			m_positionsRenderer->render();
			m_lineRenderer->render(false);
		}

		if (m_sDisplayEllipsoids) {
			m_isoellipsoidRenderer->render();
		}

		if (m_sDisplayPoints) {
			m_pointcloudRenderer->render(m_sDisplayEllipsoids);
		}

		if (!renderDensity) {
			glBlitNamedFramebuffer(m_fboIntermediate->getID(), defaultFbo, 0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		}
	}

	//Render volume if necessary
	if (renderDensity) {
		//Second pass: Pass old depth texture to ray marcher and render on screen
		glBindFramebuffer(GL_FRAMEBUFFER, defaultFbo);
		
		GLuint query;
		glGenQueries(1, &query);
		glBeginQuery(GL_TIME_ELAPSED, query);

		m_densityRenderer->render(m_fboIntermediate->getColorTexture(), m_sDisplayPoints || m_sDisplayEllipsoids || m_sDisplayGMPositions);

		glEndQuery(GL_TIME_ELAPSED);

		GLint done = 0;
		while (!done) {
			glGetQueryObjectiv(query, GL_QUERY_RESULT_AVAILABLE, &done);
		}

		GLuint64 elapsed;
		glGetQueryObjectui64v(query, GL_QUERY_RESULT, &elapsed);
		qDebug() << elapsed / 1000000.0 << "ms\n";

	}
}

void DisplayWidget::resizeGL(int width, int height)
{
	m_camera->setAspectRatio(GLfloat(width) / height);
	m_densityRenderer->setSize(width, height);
	m_fboIntermediate->setSize(width, height);
}

void DisplayWidget::mousePressEvent(QMouseEvent* event)
{
	m_lastPos = event->pos();
	if (event->modifiers().testFlag(Qt::ShiftModifier) && m_sDisplayGMPositions) {
		
		if (event->button() == Qt::RightButton) {
			emit gaussianSelected(-1);
		}
		else {
			GLint defaultFbo = 0;
			glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &defaultFbo);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fboIntermediate->getID());
			glNamedFramebufferReadBuffer(m_fboIntermediate->getID(), GL_COLOR_ATTACHMENT1);
			int width = m_fboIntermediate->getWidth();
			int height = m_fboIntermediate->getHeight();
			float* data = new float[width * height];
			glReadPixels(0, 0, width, height, GL_RED, GL_FLOAT, data);
			int glidx = (width * (height - 1 - m_lastPos.y())) + m_lastPos.x();
			int index = int(data[glidx]) - 1;
			delete[] data;
			glBindFramebuffer(GL_READ_FRAMEBUFFER, defaultFbo);
			glNamedFramebufferReadBuffer(m_fboIntermediate->getID(), GL_COLOR_ATTACHMENT0);
			//qDebug() << index << "\n";
			//Gaussian Info
			if (index >= 0) {
				qDebug() << "Selected Gaussian:\n";
				qDebug() << "  Index: " << index << "\n";
				const core::Gaussian<float>* gauss = (*m_mixture)[index];
				auto pos = gauss->getPosition();
				qDebug() << "  Position: " << pos.x() << " / " << pos.y() << " / " << pos.z() << "\n";
				qDebug() << "  Determinant: " << gauss->getCovDeterminant() << "\n";
				qDebug() << "\n";
				emit gaussianSelected(index);
			}
			else {
				qDebug() << "No Gaussian selected.\n";
			}
		}
	}
}

void DisplayWidget::mouseMoveEvent(QMouseEvent* event)
{
	if (event->modifiers().testFlag(Qt::ShiftModifier)) return;
	int dx = event->x() - m_lastPos.x();
	int dy = event->y() - m_lastPos.y();

	bool refresh = false;
	if (event->buttons() & Qt::LeftButton) {
		m_camera->rotateX(2 * dy);
		m_camera->rotateY(2 * dx);
		refresh = true;
	}
	if (event->buttons() & Qt::MiddleButton) {
		m_camera->zoom(- 2 * dy);
		refresh = true;
	}
	if (event->buttons() & Qt::RightButton) {
		m_camera->translateAlongScreen(dx, dy);
		refresh = true;
	}
	if (refresh) {
		update();
		m_lastPos = event->pos();
	}
}

//This function is copied from http://www.trentreed.net/blog/qt5-opengl-part-5-debug-logging/
void DisplayWidget::messageLogged(const QOpenGLDebugMessage& msg) {
	if (msg.id() == 131169 || msg.id() == 131185 || msg.id() == 131218 || msg.id() == 131204) {
		return;
	}
	QString error;
	
	// Format based on severity
	switch (msg.severity())
	{
	case QOpenGLDebugMessage::NotificationSeverity:
		error += "--";
		break;
	case QOpenGLDebugMessage::HighSeverity:
		error += "!!";
		break;
	case QOpenGLDebugMessage::MediumSeverity:
		error += "!~";
		break;
	case QOpenGLDebugMessage::LowSeverity:
		error += "~~";
		break;
	}

	error += " (";

	// Format based on source
#define CASE(c) case QOpenGLDebugMessage::c: error += #c; break
	switch (msg.source())
	{
		CASE(APISource);
		CASE(WindowSystemSource);
		CASE(ShaderCompilerSource);
		CASE(ThirdPartySource);
		CASE(ApplicationSource);
		CASE(OtherSource);
		CASE(InvalidSource);
	}
#undef CASE

	error += " : ";

	// Format based on type
#define CASE(c) case QOpenGLDebugMessage::c: error += #c; break
	switch (msg.type())
	{
		CASE(ErrorType);
		CASE(DeprecatedBehaviorType);
		CASE(UndefinedBehaviorType);
		CASE(PortabilityType);
		CASE(PerformanceType);
		CASE(OtherType);
		CASE(MarkerType);
		CASE(GroupPushType);
		CASE(GroupPopType);
	}
#undef CASE
	
	error += ")";
	qDebug() << qPrintable(error) << "\n" << qPrintable(msg.message()) << "\n";
}
