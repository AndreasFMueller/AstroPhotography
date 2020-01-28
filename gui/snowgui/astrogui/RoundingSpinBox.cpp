/*
 * RoundingSpinBo.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <RoundingSpinBox.h>
#include <cmath>

namespace snowgui {

RoundingSpinBox::RoundingSpinBox(QWidget *parent) : QDoubleSpinBox(parent) {
}

double	RoundingSpinBox::roundedValue() {
	return round(value() / singleStep()) * singleStep();
}

bool	RoundingSpinBox::isRounded() {
	double	tolerance = pow(10., -decimals());
	return fabs(roundedValue() - value()) < tolerance;
}

void	RoundingSpinBox::stepBy(int steps) {
	if (steps > 0) {
		while (steps--) {
			if (isRounded()) {
				setValue(value() + singleStep());
			} else {
				setValue(ceil(value() / singleStep()) * singleStep());
			}
		}
	} else {
		while (steps++) {
			if (isRounded()) {
				setValue(value() - singleStep());
			} else {
				setValue(floor(value() / singleStep()) * singleStep());
			}
		}
	}
}

} // namespace snowgui
