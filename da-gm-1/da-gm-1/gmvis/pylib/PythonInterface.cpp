
#include <pybind11/pybind11.h>
#include <QtWidgets/QApplication>
#include "gmvis/ui/VisualizerWindow.h"

namespace gmvis::pylib {

	int add(int i, int j) {
		return i + j;
	}

	void showVisualizer() {
		int argc = 0;
		char* argv = nullptr;
		QApplication a(argc, &argv);

		//gmvis::ui::VisualizerWindow w;
		//w.show();
		a.exec();
	}

	PYBIND11_MODULE(pygmvis, m) {
		m.doc() = "pybind11 example plugin";
		m.def("add", &add, "A function which adds two numbers");
		m.def("vis", &showVisualizer, "Starts the visualizer");
	}
}