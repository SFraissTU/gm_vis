#pragma once

#include <QListWidgetItem>
#include "gmvis/core/Gaussian.h"

namespace gmvis::ui {

	class GaussianListItem : public QListWidgetItem
	{
	public:
		GaussianListItem(int index, const gmvis::core::Gaussian<DECIMAL_TYPE>* gaussian);

		QString getDescription();
		int getIndex();

	private:
		int m_index;
		const gmvis::core::Gaussian<DECIMAL_TYPE>* m_gaussian;

	};

}