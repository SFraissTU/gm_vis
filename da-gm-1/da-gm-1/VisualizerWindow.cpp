#include "VisualizerWindow.h"

VisualizerWindow::VisualizerWindow(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	/*QSurfaceFormat format;
	format.setMajorVersion(4);
	format.setMinorVersion(6);
	format.setProfile(QSurfaceFormat::CoreProfile);
	format.setOption(QSurfaceFormat::DebugContext);
	QOpenGLContext* context = new QOpenGLContext;
	context->setFormat(format);
	context->create();
	QOpenGLContext::setShareContext(context);*/

	/*QOpenGLDebugLogger* logger = new QOpenGLDebugLogger(this);
	logger->initialize();
	connect(logger, &QOpenGLDebugLogger::messageLogged, this, &VisualizerWindow::glMessageLogged);*/
}/*

void VisualizerWindow::glMessageLogged(const QOpenGLDebugMessage& debugMessage)
{
	qDebug() << debugMessage;
}*/
