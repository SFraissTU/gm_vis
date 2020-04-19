#pragma once

#include "gmvis/pylib/pyimport.h"
#include <QApplication>
#include "LiveVisWindow.h"

namespace gmvis::pylib {
	class PythonLiveInterface {

	public:
		static std::shared_ptr<PythonLiveInterface> create();

	private:
		std::unique_ptr<QApplication> m_application;
		std::unique_ptr<LiveVisWindow> m_window;

	};
}