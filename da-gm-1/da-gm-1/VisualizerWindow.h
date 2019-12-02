#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_VisualizerWindow.h"

class VisualizerWindow : public QMainWindow
{
	Q_OBJECT

public:
	VisualizerWindow(QWidget *parent = Q_NULLPTR);

public:
	void glMessageLogged(const QOpenGLDebugMessage& debugMessage);

private:
	Ui::VisualizerWindowClass ui;
};
