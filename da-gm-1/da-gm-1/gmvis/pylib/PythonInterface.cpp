
#include "PythonInterface.h"
#include "gmvis/core/DataLoader.h"
#include "gmvis/core/Camera.h"
#include "gmvis/pylib/Visualizer.h"
#include <thread>
#include <QElapsedTimer>

using namespace gmvis::pylib;
using namespace gmvis::core;
using namespace pybind11::literals;

void pyprint(std::string str)
{
#ifdef PY_LIB
	py::print(str);
#else
	qDebug() << QString(str.c_str()) << "\n";
#endif
}

int main(int argc, char* argv[])
{
	Visualizer visualizer = Visualizer(true, 500, 500);
	//Visualizer visualizer = Visualizer::create(true, 500, 500);
	py::list strlist = py::list();
	strlist.append("D:/Simon/Studium/S-11 (WS19-20)/Diplomarbeit/da-gm-1/da-gm-1/data/c_30fix.ply");
	visualizer.set_gaussian_mixtures_from_paths(strlist, true);
	//visualizer.render(1);
	visualizer.finish();
	strlist.release();
	pyprint("Finished");
}

PYBIND11_MODULE(pygmvis, m) {
	m.doc() = "GM Visualizer";
	py::enum_<GMColoringRenderMode>(m, "GMColoringRenderMode")
		.value("COLOR_AMPLITUDE", GMColoringRenderMode::COLOR_AMPLITUDE)
		.value("COLOR_UNIFORM", GMColoringRenderMode::COLOR_UNIFORM)
		.value("COLOR_WEIGHT", GMColoringRenderMode::COLOR_WEIGHT)
		.export_values();
	py::enum_<GMColorRangeMode>(m, "GMColorRangeMode")
		.value("RANGE_MANUAL", GMColorRangeMode::RANGE_MANUAL)
		.value("RANGE_MINMAX", GMColorRangeMode::RANGE_MINMAX)
		.value("RANGE_MEDMED", GMColorRangeMode::RANGE_MEDMED)
        .value("RANGE_MAXABSMINMAX", GMColorRangeMode::RANGE_MAXABSMINMAX)
		.export_values();
	py::enum_<GMDensityRenderMode>(m, "GMDensityRenderMode")
		.value("ADDITIVE_EXACT", GMDensityRenderMode::ADDITIVE_EXACT)
		.value("ADDITIVE_ACC_OCTREE", GMDensityRenderMode::ADDITIVE_ACC_OCTREE)
		.value("ADDITIVE_ACC_PROJECTED", GMDensityRenderMode::ADDITIVE_ACC_PROJECTED);
	
	py::class_<Visualizer, std::shared_ptr<Visualizer>>(m, "Visualizer")
		.def("set_image_size", &Visualizer::set_image_size, "width"_a, "height"_a)
		.def("set_camera_auto", &Visualizer::set_camera_auto, "mode"_a)
		.def("set_camera_lookat", &Visualizer::set_camera_lookat, "position"_a, "lookat"_a, "up"_a)
		.def("set_view_matrix", &Visualizer::set_view_matrix, "viewmat"_a)
		.def("set_ellipsoids_rendering", &Visualizer::set_ellipsoids_rendering, "ellipsoids"_a, "pointcloud"_a = true)
		.def("set_ellipsoids_colormode", &Visualizer::set_ellipsoids_colormode, "colorMode"_a = GMColoringRenderMode::COLOR_UNIFORM)
		.def("set_ellipsoids_rangemode", &Visualizer::set_ellipsoids_rangemode, "rangeMode"_a = GMColorRangeMode::RANGE_MANUAL, "min"_a=1.0f, "max"_a=0.0f)
		.def("set_positions_rendering", &Visualizer::set_positions_rendering, "positions"_a, "pointcloud"_a = true)
		.def("set_positions_colormode", &Visualizer::set_positions_colormode, "colorMode"_a = GMColoringRenderMode::COLOR_UNIFORM)
		.def("set_positions_rangemode", &Visualizer::set_positions_rangemode, "rangeMode"_a = GMColorRangeMode::RANGE_MANUAL, "min"_a=1.0f, "max"_a=0.0f)
		.def("set_density_rendering", &Visualizer::set_density_rendering, "density"_a = true)
		.def("set_density_rendermode", &Visualizer::set_density_rendermode, "renderMode"_a = GMDensityRenderMode::ADDITIVE_EXACT)
		.def("set_density_range_auto", &Visualizer::set_density_range_auto, "autoperc"_a = 0.75)
		.def("set_density_range_manual", &Visualizer::set_density_range_manual, "min"_a, "max"_a)
		.def("set_density_logarithmic", &Visualizer::set_density_logarithmic, "logarithmic"_a)
		.def("set_density_accthreshold", &Visualizer::set_density_accthreshold, "automatic"_a = true, "threshold"_a = 0.00001)
		.def("set_pointclouds", &Visualizer::set_pointclouds, "pointclouds"_a)
		.def("set_pointclouds_from_paths", &Visualizer::set_pointclouds_from_paths, "paths"_a)
		.def("set_gaussian_mixtures", &Visualizer::set_gaussian_mixtures, "mixtures"_a, "isgmm"_a)
		.def("set_gaussian_mixtures_from_paths", &Visualizer::set_gaussian_mixtures_from_paths, "paths"_a, "isgmm"_a)
		.def("set_callback", &Visualizer::set_callback, "callback"_a)
		.def("render", &Visualizer::render, "epoch"_a)
		.def("finish", &Visualizer::finish)
		.def("forceStop", &Visualizer::forceStop);

	m.def("create_visualizer", &Visualizer::create, "async"_a, "width"_a, "height"_a);
}
