#include "PointCloudLoader.h"
#include <qfile.h>
#include <qtextstream.h>
#include <qdebug.h>

std::vector<QVector3D> PointCloudLoader::readPCDfromOFF(QString path, bool switchYandZ)
{
	QFile file(path);
	if (file.open(QIODevice::ReadOnly)) {
		QTextStream in(&file);
		//Check Format
		QString line = in.readLine();
		if (line != "OFF") {
			qDebug() << "Invalid format in file " << path << "\n";
			return {};
		}
		line = in.readLine();
		bool ok;
		int amount = line.split(" ").at(0).toInt(&ok);
		if (!ok) {
			qDebug() << "Could not read amount of points in file " << path << "\n";
			return {};
		}
		std::vector<QVector3D> result;
		result.reserve(amount);
		while (!in.atEnd() && result.size() != amount) {
			line = in.readLine();
			QStringList numbers = line.split(" ");
			if (numbers.size() != 3) {
				qDebug() << "Invalid number of coordinates given in line " << (result.size() + 2) << " of file " << path << "\n";
				return {};
			}
			float px = numbers[0].toFloat(&ok);
			if (!ok) {
				qDebug() << "Invalid coordinates in line " << (result.size() + 2) << " of file " << path << "\n";
				return {};
			}
			float py = numbers[1].toFloat(&ok);
			if (!ok) {
				qDebug() << "Invalid coordinates in line " << (result.size() + 2) << " of file " << path << "\n";
				return {};
			}
			float pz = numbers[2].toFloat(&ok);
			if (!ok) {
				qDebug() << "Invalid coordinates in line " << (result.size() + 2) << " of file " << path << "\n";
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
			qDebug() << "Number of points invalid in file " << path << "\n";
		}
		return result;
	}
	else {
		qDebug() << "Could not open file: " << path << "\n";
		return {};
	}
}
