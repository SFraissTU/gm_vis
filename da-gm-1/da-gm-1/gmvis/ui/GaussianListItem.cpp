#include "GaussianListItem.h"

using namespace gmvis::ui;
using namespace gmvis::core;

GaussianListItem::GaussianListItem(int index, const Gaussian<DECIMAL_TYPE>* gaussian)
	: m_index(index), 
	m_gaussian(gaussian)
{
	std::stringstream stream;
	auto pos = m_gaussian->getPosition();
	stream << "#" << m_index << ": " << pos[0] << ", " << pos[1] << ", " << pos[2];
	setText(QString::fromStdString(stream.str()));
}

int GaussianListItem::getIndex()
{
	return m_index;
}

QString GaussianListItem::getDescription()
{
	std::stringstream stream;
	stream << "Selected Gaussian: #" << m_index << "\n";
	auto pos = m_gaussian->getPosition();
	stream << "  Position:\n";
	Eigen::IOFormat formatP(8, 0, " ", ", ");
	stream << "    " << pos.format(formatP) << "\n";
	stream << "  Covariance:\n";
	auto cov = m_gaussian->getCovarianceMatrix();
	Eigen::IOFormat format(5, 0, " ", "\n", "    ");
	stream << cov.format(format) << "\n";
	stream << "  Weight: " << m_gaussian->getNormalizedWeight() << "\n";
	stream << "  Cov Determinant: " << m_gaussian->getCovDeterminant() << "\n";
	stream << "  Amplitude: " << m_gaussian->getAmplitude() << "\n";
	return QString::fromStdString(stream.str());
}
