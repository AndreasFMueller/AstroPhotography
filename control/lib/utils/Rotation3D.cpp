/*
 * Rotation3D.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroCoordinates.h>
#include <AstroDebug.h>

namespace astro {

void	Rotation3D::setup(const UnitVector& u, const Angle& a) {
	double	s = sin(a);
	double	c = cos(a);

	m[0][0] = c + (1-c) * u._x[0] * u._x[0];
	m[0][1] = (1-c) * u._x[0] * u._x[1] - s * u._x[2];
	m[0][2] = (1-c) * u._x[0] * u._x[2] - s * u._x[1];

	m[1][0] = (1-c) * u._x[0] * u._x[1] - s * u._x[2];
	m[1][1] = c + (1-c) * u._x[1] * u._x[1];
	m[1][2] = (1-c) * u._x[1] * u._x[2] - s * u._x[0];

	m[2][0] = (1-c) * u._x[0] * u._x[2] - s * u._x[1];
	m[2][1] = (1-c) * u._x[1] * u._x[2] - s * u._x[0];
	m[2][2] = c + (1-c) * u._x[2] * u._x[2];
}

Rotation3D::Rotation3D(const Vector& a) {
	setup(UnitVector(a), Angle(a.abs(), Angle::Radians));
}

Rotation3D::Rotation3D(const UnitVector& u, const Angle& a) {
	setup(u, a);
}

Rotation3D::Rotation3D(char axis, const Angle& a) {
	switch (axis) {
	case 'x':
		setup(UnitVector::Ex(), a);
		break;
	case 'y':
		setup(UnitVector::Ey(), a);
		break;
	case 'z':
		setup(UnitVector::Ez(), a);
		break;
	default:
		{
		std::string	msg = stringprintf("unknown axis '%c'", axis);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
		}
	}
}

Vector	Rotation3D::operator()(const Vector& v) const {
	double	y[3];
	for (int i = 0; i < 3; i++) {
		y[i] = 0.;
		for (int k = 0; k < 3; k++) {
			y[i] += m[i][k] * v._x[k];
		}
	}
	return Vector(y);
}

UnitVector	Rotation3D::operator()(const UnitVector& v) const {
	double	y[3];
	for (int i = 0; i < 3; i++) {
		y[i] = 0.;
		for (int k = 0; k < 3; k++) {
			y[i] += m[i][k] * v._x[k];
		}
	}
	return UnitVector(y);
}

} // namespace astro
