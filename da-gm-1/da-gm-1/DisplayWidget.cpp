#include "DisplayWidget.h"

#include <memory>

#include "math.h"

//Partialy used this: https://code.qt.io/cgit/qt/qtbase.git/tree/examples/opengl/hellogl2?h=5.13

DisplayWidget::DisplayWidget(QWidget* parent) : QOpenGLWidget(parent) {
	QSurfaceFormat format;
	format.setMajorVersion(4);
	format.setMinorVersion(6);
	format.setProfile(QSurfaceFormat::CoreProfile);
	format.setOption(QSurfaceFormat::DebugContext);
	setFormat(format);
	m_camera = std::make_unique<Camera>(60.0f, GLfloat(width()) / height(), 0.01f, 1000.0f);
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

//should only be called after initialization!
void DisplayWidget::setPointCloud(PointCloud* pointcloud)
{
	m_pointcloudRenderer->setPointCloud(pointcloud);
	update();
}

void DisplayWidget::setGaussianMixture(GaussianMixture* mixture)
{
	m_isoellipsoidRenderer->setMixture(mixture);
	//m_densityRendererTexSampled->setMixture(mixture);
	//m_densityRendererDirectSampled->setMixture(mixture);
	m_densityRendererAnalyticAdd->setMixture(mixture);
	update();
}

void DisplayWidget::cleanup() {
	makeCurrent();
	//This could be called several times, so make sure things are not deleted already
	if (m_pointcloudRenderer.get()) {
		m_pointcloudRenderer->cleanup();
		m_pointcloudRenderer.reset();
		m_isoellipsoidRenderer->cleanup();
		m_isoellipsoidRenderer.reset();
		m_densityRendererTexSampled->cleanup();
		m_densityRendererTexSampled.reset();
		m_debugLogger.reset();
	}
	doneCurrent();
}

void DisplayWidget::initializeGL() {
	connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &DisplayWidget::cleanup);

	initializeOpenGLFunctions();
	m_fboIntermediate = std::make_unique<ScreenFBO>(static_cast<QOpenGLFunctions_4_5_Core*>(this), width(), height(), true);
	m_fboIntermediate->attachColorTexture();
	m_fboIntermediate->attachDepthTexture();

#if _DEBUG
	m_debugLogger = std::make_unique<QOpenGLDebugLogger>(this);
	if (m_debugLogger->initialize()) {
		qDebug() << "GL_DEBUG Debug Logger " << m_debugLogger.get() << "\n";
		connect(m_debugLogger.get(), &QOpenGLDebugLogger::messageLogged, this, &DisplayWidget::messageLogged);
		m_debugLogger->startLogging();
	}
#endif

	m_pointcloudRenderer = std::make_unique<PointCloudRenderer>(static_cast<QOpenGLFunctions_4_5_Core*>(this), &m_settings, m_camera.get());
	m_isoellipsoidRenderer = std::make_unique<GMIsoellipsoidRenderer>(static_cast<QOpenGLFunctions_4_5_Core*>(this), &m_settings, m_camera.get());
	m_densityRendererTexSampled = std::make_unique<GMDensityRendererTexSampled>(static_cast<QOpenGLFunctions_4_5_Core*>(this), &m_settings, m_camera.get(), width(), height());
	m_densityRendererDirectSampled = std::make_unique<GMDensityRendererDirectSampled>(static_cast<QOpenGLFunctions_4_5_Core*>(this), &m_settings, m_camera.get(), width(), height());
	m_densityRendererAnalyticAdd = std::make_unique<GMDensityRendererAnalyticAdd>(static_cast<QOpenGLFunctions_4_5_Core*>(this), &m_settings, m_camera.get(), width(), height());

	auto background = m_settings.backgroundColor;
	glClearColor(background.redF(), background.blueF(), background.greenF(), 1);
}

void DisplayWidget::paintGL()
{
	GLint defaultFbo = 0;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &defaultFbo);
	//First pass: Render classicaly into texture
	glBindFramebuffer(GL_FRAMEBUFFER, m_fboIntermediate->getID());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	m_pointcloudRenderer->render();
	m_isoellipsoidRenderer->render();

	//Second pass: Pass old depth texture to ray marcher and render on screen
	glBindFramebuffer(GL_FRAMEBUFFER, defaultFbo);
	//m_densityRendererTexSampled->render();
	//m_densityRendererDirectSampled->render();
	m_densityRendererAnalyticAdd->render(m_fboIntermediate->getDepthTexture());
}

void DisplayWidget::resizeGL(int width, int height)
{
	m_camera->setAspectRatio(GLfloat(width) / height);
	m_densityRendererTexSampled->setSize(width, height);
	m_densityRendererDirectSampled->setSize(width, height);
	m_densityRendererAnalyticAdd->setSize(width, height);
	m_fboIntermediate->setSize(width, height);
}

void DisplayWidget::mousePressEvent(QMouseEvent* event)
{
	m_lastPos = event->pos();
}

void DisplayWidget::mouseMoveEvent(QMouseEvent* event)
{
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
