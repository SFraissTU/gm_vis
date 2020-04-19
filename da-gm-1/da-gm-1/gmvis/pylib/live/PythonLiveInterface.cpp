#include "PythonLiveInterface.h"

using namespace gmvis::pylib;

std::shared_ptr<PythonLiveInterface> PythonLiveInterface::create()
{
	std::shared_ptr<PythonLiveInterface> obj = std::make_shared<PythonLiveInterface>();
	int argc = 0;
	obj->m_application = std::make_unique<QApplication>(argc, nullptr);
	obj->m_window = std::make_unique<LiveVisWindow>();
	obj->m_window->show();
	obj->m_application->exec();
	return obj;
}

PYBIND11_MODULE(pygmvislive, m) {
	m.doc() = "GM Visualizer Live";

	py::class_<PythonLiveInterface, std::shared_ptr<PythonLiveInterface>>(m, "LiveInterface");

	m.def("create", &PythonLiveInterface::create);
}