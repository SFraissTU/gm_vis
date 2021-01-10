#include "DataLoader.h"
#include <qfile.h>
#include <qtextstream.h>
#include <qdebug.h>
#include <QColor>
#include <QCoreApplication>

using namespace gmvis::core;

QMap<QString, QString> gmvis::core::DataLoader::readConfigFile(const QString& path)
{
	QMap<QString, QString> map;
	QFile file("res/config.txt");
	if (!file.exists())
		file.setFileName(QCoreApplication::applicationDirPath() + "/res/config.txt");
	if (file.open(QIODevice::ReadOnly)) {
		QTextStream in(&file);
		while (!in.atEnd()) {
			QString line = in.readLine();
			int idx = line.indexOf("=");
			if (idx != -1) {
				map.insert(line.left(idx), line.right(line.length() - idx - 1));
			}
		}
		file.close();
	}
	return map;
}

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

template <typename decimal>
std::unique_ptr<GaussianMixture<decimal>> DataLoader::readGMfromPLY(QFile& file, bool isgmm, bool convertCoordinateSystem)
{
	if (file.open(QIODevice::ReadOnly)) {
		QTextStream in(&file);

		QString line = in.readLine();
		if (line.toUpper() != "PLY") {
			qDebug() << "Invalid format in file " << file.fileName() << "\n";
			return {};
		}
		std::vector<RawGaussian<decimal>> rawGaussians;
		int numberOfGaussians = 0;
		int remainingGaussians = 0;
		QList<QString> properties;
		QList<QString> supportedProperties = {"x", "y", "z", "covxx", "covxy", "covxz", "covyy", "covyz", "covzz", "weight"};
		QList<QString> ignoredProperties = { "nvx", "nvy", "nvz" };
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
				if ((words[1] == "float" || words[1] == "double") && supportedProperties.contains(words[2])) {
					properties.append(words[2]);
					if (words[1] == "float" && typeid(decimal).name() != "float")
					{
						qDebug() << "Warning: Reading float as double.\n";
					}
					else if (words[1] == "double" && typeid(decimal).name() != "double")
					{
						qDebug() << "Warning: Reading double as float.\n";
					}
				}
				else {
					if (!ignoredProperties.contains(words[2])) {
						qDebug() << "Unknown property " << words[2] << " in ply file " << file.fileName() << ". Ignoring.\n";
					}
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
			RawGaussian<decimal> newGaussian;
			for (int i = 0; i < words.length(); ++i) {
				if (!properties[i].isNull()) {
					bool ok;
					decimal val = strToDecimal<decimal>(words[i], &ok);
					if (!ok) {
						qDebug() << "A value could not be transformed into a decimal in ply file " << file.fileName() << "\n.";
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
			rawGaussians.push_back(newGaussian);
			--remainingGaussians;
		}
		if (remainingGaussians > 0) {
			qDebug() << "End of file was reached before all Gaussians were read in file " << file.fileName() << ".\n";
			return {};
		}
		//The weights are multiplied by the amount of points, so we have to normalize them
		if (isgmm) {
			RawGaussian<decimal>::normalize(rawGaussians);
		}
		std::unique_ptr<GaussianMixture<decimal>> mixture = std::make_unique<GaussianMixture<decimal>>(rawGaussians, isgmm);
		qDebug() << "Successfully read " << mixture->numberOfGaussians() << " Gaussians from " << file.fileName() << ".\n";
		if (!mixture->isValid())
		{
			qDebug() << "Warning: Mixture is not valid!\n";
		}
		return std::move(mixture);
	}
	else {
		qDebug() << "Could not open file: " << file.fileName() << "\n";
	}
	return std::unique_ptr<GaussianMixture<decimal>>();
}

template std::unique_ptr<GaussianMixture<float>> DataLoader::readGMfromPLY(QFile& file, bool isgmm, bool convertCoordinateSystem);
template std::unique_ptr<GaussianMixture<double>> DataLoader::readGMfromPLY(QFile& file, bool isgmm, bool convertCoordinateSystem);

template <typename decimal>
std::unique_ptr<GaussianMixture<decimal>> DataLoader::readGMfromPLY(const QString& path, bool isgmm, bool convertCoordinateSystem)
{
	QFile file(path);
	return readGMfromPLY<decimal>(file, isgmm, convertCoordinateSystem);
}

template std::unique_ptr<GaussianMixture<float>> DataLoader::readGMfromPLY(const QString& path, bool isgmm, bool convertCoordinateSystem);
template std::unique_ptr<GaussianMixture<double>> DataLoader::readGMfromPLY(const QString& path, bool isgmm, bool convertCoordinateSystem);

template<> 
double gmvis::core::DataLoader::strToDecimal<double>(QString str, bool* ok)
{
	return str.toDouble(ok);
}

template<> 
float gmvis::core::DataLoader::strToDecimal<float>(QString str, bool* ok)
{
	return str.toFloat(ok);
}

std::unique_ptr<LineStrip> gmvis::core::DataLoader::readLSfromTXT(const QString& path)
{
	QFile file(path);
	if (file.open(QIODevice::ReadOnly)) {
		QTextStream in(&file);
		point_list vlist;
		//Check Format
		QString line = in.readLine();
		while (!line.isNull()) {
			QStringList list = line.split(" ");
			double x = list[0].toDouble();
			double y = list[1].toDouble();
			double z = list[2].toDouble();
			vlist.push_back(point_item(x, y, z));
			line = in.readLine();
		}
		file.close();
		return std::make_unique<LineStrip>(vlist);
	}
	return nullptr;

}

std::unique_ptr<LineStrip> gmvis::core::DataLoader::readLSfromBIN(const QString& path)
{
	QFile file(path);
	if (file.open(QIODevice::ReadOnly)) {
		unsigned int i = 1;
		char* c = (char*)&i;
		bool littleendian = *c;

		point_list vlist;
		//Check Format
		char* arr = new char[3 * 8];
		qint64 read = file.read(arr, 3*8);
		while (read == 3*8) {
			double x, y, z;
			if (littleendian) {
				memcpy(&x, &arr[0], 8);
				memcpy(&y, &arr[8], 8);
				memcpy(&z, &arr[16], 8);
			}
			else {
				char* xx = (char*)&x;
				char* yy = (char*)&y;
				char* zz = (char*)&z;
				memcpy(xx, &arr[4], 4);
				memcpy(&xx[4], &arr[0], 4);
				memcpy(yy, &arr[12], 4);
				memcpy(&yy[4], &arr[8], 4);
				memcpy(zz, &arr[20], 4);
				memcpy(&zz[4], &arr[16], 4);
			}
			vlist.push_back(point_item(x, y, z));
			read = file.read(arr, 3 * 8);
		}
		file.close();
		return std::make_unique<LineStrip>(vlist);
	}
	return nullptr;
}

QVector<QVector3D> DataLoader::readTransferFunction(const QString& path)
{
	QVector<QVector3D> result;
	QFile file(":/" + path);
	if (QFile::exists(path)) {
		file.setFileName(path);
	}
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

QByteArray DataLoader::readRessource(const QString& path)
{
	QFile f(":/" + path);
	if (QFile::exists(path)) {
		f.setFileName(path);
	}

	return f.open(QIODevice::ReadOnly) ? f.readAll() : "";
}
