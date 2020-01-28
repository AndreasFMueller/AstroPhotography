/*
 * RoundingSpinBox.h -- SpinBox with rounding
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _RoundingSpinBox_h
#define _RoundingSpinBox_h

#include <QDoubleSpinBox>

namespace snowgui {

class RoundingSpinBox : public QDoubleSpinBox {
	Q_OBJECT
	double	roundedValue();
	bool	isRounded();
public:
	explicit RoundingSpinBox(QWidget *parent = NULL);
	virtual void	stepBy(int steps);
};

}

#endif /* _RoundingSpinBox */
