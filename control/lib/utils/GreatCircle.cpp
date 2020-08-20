/*
 * GreatCircle.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroCoordinates.h>
#include <cmath>

namespace astro {

GreatCircle::GreatCircle(const RaDec& A, const RaDec& B) : _A(A), _B(B) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "great circle from %s to %s",
		_A.toString().c_str(), _B.toString().c_str());
	_gamma = B.ra() - A.ra();
	_sign = 1;
	float	_delta = _gamma.radians();
	if ((-2 * M_PI < _delta) && (_delta <= -M_PI)) {
		_gamma = Angle(2 * M_PI + _delta, Angle::Radians);
		_sign = 1;
	}
	if ((-M_PI < _delta) && (_delta <= 0)) {
		_gamma = Angle(_delta, Angle::Radians);
		_sign = -1;
	}
	if ((0 < _delta) && (_delta <= M_PI)) {
		_gamma = Angle(_delta, Angle::Radians);
		_sign = 1;
	}
	if ((M_PI < _delta) && (_delta <= 2 * M_PI)) {
		_gamma = Angle(2 * M_PI - _delta, Angle::Radians);
		_sign = -1;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "gamma = %s", _gamma.dms().c_str());
	_a = Angle::right_angle - B.dec();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "a = %s", _a.dms().c_str());
	_b = Angle::right_angle - A.dec();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "b = %s", _b.dms().c_str());
	_c = arccos(cos(_a)*cos(_b) + sin(_a)*sin(_b)*cos(_gamma));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "c = %s", _c.dms().c_str());
	_alpha = arccos((cos(_a) - cos(_b) * cos(_c))/(sin(_b) * sin(_c)));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "alpha = %s", _alpha.dms().c_str());
	_beta = arccos((cos(_b) - cos(_c) * cos(_a))/(sin(_c) * sin(_a)));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "beta = %s", _beta.dms().c_str());
}

Angle	GreatCircle::c(double t) const {
	Angle	c = t * _c;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "c(%.2f) = %s", t, c.dms().c_str());
	return c;
}

Angle	GreatCircle::a(double t) const {
	Angle	a = arccos(cos(_b) * cos(c(t))
			+ sin(_b) * sin(c(t)) * cos(_alpha));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "a(%.2f) = %s", t, a.dms().c_str());
	return a;
}

Angle	GreatCircle::gamma(double t) const {
	Angle	gamma = arccos((cos(c(t)) - cos(_b) * cos(a(t)))
				/ (sin(_b) * sin(a(t))));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "gamma(%.2f) = %s", t,
		gamma.dms().c_str());
	return gamma;
}

RaDec	GreatCircle::operator()(double t) const {
	Angle	ra = _A.ra() + _sign * gamma(t);
	ra.reduce();
	RaDec result(ra, Angle::right_angle - a(t));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "point %s", result.toString().c_str());
	return result;
}

} // namespace astro
