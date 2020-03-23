#include "VisualizerWindow.h"
#include <QtWidgets/QApplication>

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);
	
	gmvis::ui::VisualizerWindow w;
	w.show();
	return a.exec();
}