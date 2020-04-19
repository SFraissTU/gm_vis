#include "pyimport.h"

void pyprint(std::string str)
{
#ifdef PY_LIB
	py::print(str);
#else
	qDebug() << QString(str.c_str()) << "\n";
#endif
}