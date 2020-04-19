#pragma once

#include <memory>
#include <QtWidgets/QMainWindow>
#include "gmvis/ui/VisualizerWindow.h"
#include "gmvis/pylib/pyimport.h"
#include <QApplication>

namespace gmvis::pylib {

	class LiveVisWindow : public gmvis::ui::VisualizerWindow
	{
		Q_OBJECT

	public:
		LiveVisWindow(QWidget* parent = Q_NULLPTR);

	private:
		static std::unique_ptr<QApplication> m_application;
	};
}