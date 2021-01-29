#pragma once
#include "PointCloud.h"
#include "GaussianMixture.h"
#include "LineStrip.h"
#include <qstring.h>
#include <qfile.h>
#include <memory>

//Convert Coordinate System: (x,y,z) -> (x,z,-y)

namespace gmvis::core {

	class DataLoader {
	public:
		static QMap<QString, QString> readConfigFile(const QString& path);

		static std::unique_ptr<PointCloud> readPCDfromOFF(QFile& file, bool convertCoordinateSystem, QString& error);
		static std::unique_ptr<PointCloud> readPCDfromOFF(const QString& path, bool convertCoordinateSystem, QString& error);

		template <typename decimal>
		static std::unique_ptr<GaussianMixture<decimal>> readGMfromPLY(QFile& file, bool isgmm, bool convertCoordinateSystem, QString& error);
		template <typename decimal>
		static std::unique_ptr<GaussianMixture<decimal>> readGMfromPLY(const QString& path, bool isgmm, bool convertCoordinateSystem, QString& error);

		static std::unique_ptr<LineStrip> readLSfromTXT(const QString& path);
		static std::unique_ptr<LineStrip> readLSfromBIN(const QString& path);

		static QVector<QVector3D> readTransferFunction(const QString& path);

		static QByteArray readRessource(const QString& path);

	private:
		template <typename decimal>
		static decimal strToDecimal(QString str, bool* ok);
	};

}
