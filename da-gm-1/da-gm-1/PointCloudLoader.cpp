#include "PointCloudLoader.h"
#include <qfile.h>
#include <qtextstream.h>
#include <qdebug.h>
std::unique_ptr<PointCloud> PointCloudLoader::readPCDfromOFF(QFile& file, bool switchYandZ)
{
	if (file.open(QIODevice::ReadOnly)) {
		QTextStream in(&file);
		//Check Format
		QString line = in.readLine();
		if (line != "OFF") {
			qDebug() << "Invalid format in file " << file.fileName() << "\n";
			return {};
		}
		line = in.readLine();
		bool ok;
		int amount = line.split(" ").at(0).toInt(&ok);
		if (!ok) {
			qDebug() << "Could not read amount of points in file " << file.fileName() << "\n";
			return {};
		}
		point_list result;
		result.reserve(amount);
		while (!in.atEnd() && result.size() != amount) {
			line = in.readLine();
			QStringList numbers = line.split(" ");
			if (numbers.size() != 3) {
				qDebug() << "Invalid number of coordinates given in line " << (result.size() + 2) << " of file " << file.fileName() << "\n";
				return {};
			}
			float px = numbers[0].toFloat(&ok);
			if (!ok) {
				qDebug() << "Invalid coordinates in line " << (result.size() + 2) << " of file " << file.fileName() << "\n";
				return {};
			}
			float py = numbers[1].toFloat(&ok);
			if (!ok) {
				qDebug() << "Invalid coordinates in line " << (result.size() + 2) << " of file " << file.fileName() << "\n";
				return {};
			}
			float pz = numbers[2].toFloat(&ok);
			if (!ok) {
				qDebug() << "Invalid coordinates in line " << (result.size() + 2) << " of file " << file.fileName() << "\n";
				return {};
			}
			if (switchYandZ) {
				result.emplace_back(px, pz, -py);
			}
			else {
				result.emplace_back(px, py, pz);
			}
		}
		if (result.size() != amount) {
			qDebug() << "Number of points invalid in file " << file.fileName() << "\n";
		}
		return std::move(std::make_unique<PointCloud>(result));	//ToDo: is move necessary?
	}
	else {
		qDebug() << "Could not open file: " << file.fileName() << "\n";
		return nullptr;
	}
}

std::unique_ptr<PointCloud> PointCloudLoader::readPCDfromOFF(QString path, bool switchYandZ)
{
	QFile file(path);
	return readPCDfromOFF(file, switchYandZ);
}
