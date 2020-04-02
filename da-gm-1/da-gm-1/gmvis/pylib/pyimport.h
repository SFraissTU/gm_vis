#pragma once

#pragma push_macro("slots")
#undef slots
#include <pybind11/pybind11.h>
#include <pybind11\include\pybind11\numpy.h>
#pragma pop_macro("slots")
#include <qdebug.h>
#include <QString>
namespace py = pybind11;



void pyprint(std::string str);