#include "VisualizerWindow.h"
#include <QtWidgets/QApplication>

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);
	
	gmvis::ui::VisualizerWindow w;
	w.show();

    if (a.arguments().size() == 2) {
        w.loadPureMixture(a.arguments().back());
    }
	return a.exec();
}