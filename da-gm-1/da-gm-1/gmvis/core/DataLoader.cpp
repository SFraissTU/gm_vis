#include "DataLoader.h"
#include <qfile.h>
#include <qtextstream.h>
#include <qdebug.h>
#include <QColor>

using namespace gmvis::core;

std::unique_ptr<PointCloud> DataLoader::readPCDfromOFF(QFile& file, bool convertCoordinateSystem)
{
	if (file.open(QIODevice::ReadOnly)) {
		QTextStream in(&file);
		//Check Format
		QString line = in.readLine();
		if (line.toUpper() != "OFF") {
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
			if (convertCoordinateSystem) {
				result.push_back(point_item(px, pz, -py));
			}
			else {
				result.push_back(point_item(px, py, pz));
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

std::unique_ptr<PointCloud> DataLoader::readPCDfromOFF(const QString& path, bool convertCoordinateSystem)
{
	QFile file(path);
	return readPCDfromOFF(file, convertCoordinateSystem);
}

std::unique_ptr<GaussianMixture> DataLoader::readGMfromPLY(QFile& file, bool convertCoordinateSystem)
{
	if (file.open(QIODevice::ReadOnly)) {
		QTextStream in(&file);

		QString line = in.readLine();
		if (line.toUpper() != "PLY") {
			qDebug() << "Invalid format in file " << file.fileName() << "\n";
			return {};
		}
		std::unique_ptr<GaussianMixture> mixture = std::make_unique<GaussianMixture>();
		int numberOfGaussians = 0;
		int remainingGaussians = 0;
		QList<QString> properties;
		QList<QString> supportedProperties = {"x", "y", "z", "covxx", "covxy", "covxz", "covyy", "covyz", "covzz", "weight"};
		//Process Header
		bool processing_header = true;
		while (processing_header && !in.atEnd()) {
			line = in.readLine();
			QStringList words = line.split(" ");
			QString type = words[0];
			if (type == "format") {
				if (words.length() < 3 || words[1] != "ascii" || words[2] != "1.0") {
					qDebug() << "Format in ply file " << file.fileName() << " not supported. Only supports ascii 1.0. \n";
					return {};
				}
			}
			else if (type == "element") {
				if (words.length() < 2 || words[1] != "component") {
					qDebug() << "Invalid element type in ply file " << file.fileName() << ". Only supports component.\n";
					return {};
				}
				if (words.length() < 3) {
					qDebug() << "Invalid format: No component count given in ply file " << file.fileName() << ".\n";
					return {};
				}
				bool ok; 
				int number = words[2].toInt(&ok);
				if (!ok) {
					qDebug() << "Invalid number of components given in ply file " << file.fileName() << ".\n";
					return {};
				}
				numberOfGaussians = number;
				remainingGaussians = number;
			}
			else if (type == "property") {
				if (words.length() != 3) {
					qDebug() << "Invalid property specification in ply file " << file.fileName() << ".\n";
					return {};
				}
				if (words[1] == "float" && supportedProperties.contains(words[2])) {
					properties.append(words[2]);
				}
				else {
					qDebug() << "Unknown property " << words[2] << " in ply file " << file.fileName() << ". Ignoring.\n";
					properties.append(QString());
				}
			}
			else if (type == "end_header") {
				processing_header = false;
				//Check if all necessary properties are specified
				for (auto it = supportedProperties.constBegin(); it != supportedProperties.constEnd(); ++it) {
					if (!properties.contains(*it)) {
						qDebug() << "No property definition given for property " << *it << " in file " << file.fileName() << ".\n";
					}
				}
			}
			else if (type != "comment") {
				qDebug() << "Unknown command in ply file " << file.fileName() << ": " << type << ". Ignoring.\n";
			}
		}
		if (processing_header) {
			qDebug() << "File was ended before finished reading file " << file.fileName() << "\n.";
			return {};
		}
		while (remainingGaussians > 0 && !in.atEnd()) {
			line = in.readLine();
			QStringList words = line.split("  ");
			words.removeAll(QString(""));
			if (words.length() != properties.length()) {
				qDebug() << "Not every property was given for an entry in ply file " << file.fileName() << "\n.";
				return {};
			}
			Gaussian newGaussian;
			for (int i = 0; i < words.length(); ++i) {
				if (!properties[i].isNull()) {
					bool ok;
					double val = words[i].toDouble(&ok);
					if (!ok) {
						qDebug() << "A value could not be transformed into a float in ply file " << file.fileName() << "\n.";
						return {};
					}
					if (properties[i] == "x") {
						newGaussian.mux = val;
					}
					else if (properties[i] == "y") {
						if (convertCoordinateSystem) {
							newGaussian.muz = -val;
						}
						else {
							newGaussian.muy = val;
						}
					}
					else if (properties[i] == "z") {
						if (convertCoordinateSystem) {
							newGaussian.muy = val;
						}
						else {
							newGaussian.muz = val;
						}
					}
					else if (properties[i] == "covxx") {
						newGaussian.covxx = val;
					}
					else if (properties[i] == "covxy") {
						if (convertCoordinateSystem) {
							newGaussian.covxz = -val;
						}
						else {
							newGaussian.covxy = val;
						}
					}
					else if (properties[i] == "covxz") {
						if (convertCoordinateSystem) {
							newGaussian.covxy = val;
						}
						else {
							newGaussian.covxz = val;
						}
					}
					else if (properties[i] == "covyy") {
						if (convertCoordinateSystem) {
							newGaussian.covzz = val;
						}
						else {
							newGaussian.covyy = val;
						}
					}
					else if (properties[i] == "covyz") {
						if (convertCoordinateSystem) {
							newGaussian.covyz = -val;
						} else {
							newGaussian.covyz = val;
						}
					}
					else if (properties[i] == "covzz") {
						if (convertCoordinateSystem) {
							newGaussian.covyy = val;
						}
						else {
							newGaussian.covzz = val;
						}
					}
					else if (properties[i] == "weight") {
						newGaussian.weight = val;
					}
				}
			}
			newGaussian.finalizeInitialization();
			mixture->addGaussian(newGaussian);
			--remainingGaussians;
		}
		if (remainingGaussians > 0) {
			qDebug() << "End of file was reached before all Gaussians were read in file " << file.fileName() << ".\n";
			return {};
		}
		//The weights are multiplied by the amount of points, so we have to normalize them
		mixture->normalize();
		qDebug() << "Successfully read " << mixture->numberOfGaussians() << " Gaussians from " << file.fileName() << ".\n";
		return std::move(mixture);
	}
	else {
		qDebug() << "Could not open file: " << file.fileName() << "\n";
	}
	return std::unique_ptr<GaussianMixture>();
}

std::unique_ptr<GaussianMixture> DataLoader::readGMfromPLY(const QString& path, bool convertCoordinateSystem)
{
	QFile file(path);
	return readGMfromPLY(file, convertCoordinateSystem);
}

QVector<QVector3D> DataLoader::readTransferFunction(const QString& path)
{
	QVector<QVector3D> result;
	QFile file(path);
	if (file.open(QIODevice::ReadOnly)) {
		QTextStream in(&file);
		while (!in.atEnd()) {
			QString line = in.readLine();
			QStringList vals = line.split(",");
			if (vals.length() != 3) {
				continue;
			}
			QVector3D c = QVector3D(vals[0].toFloat(), vals[1].toFloat(), vals[2].toFloat());
			result.push_back(c);
		}
	}
	return result;
}
