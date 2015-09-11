/*
 * PointSpreadFunction.cpp
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroChart.h>

namespace astro {
namespace catalog {

//////////////////////////////////////////////////////////////////////
// DiracPointSpreadFunction
//////////////////////////////////////////////////////////////////////
double	DiracPointSpreadFunction::operator()(double r) const {
	if (r == 0) {
		return 1.;
	}
	return 0.;
}

//////////////////////////////////////////////////////////////////////
// CirclePointSpreadFunction class implementation
//////////////////////////////////////////////////////////////////////
double	CirclePointSpreadFunction::operator()(double r) const {
	if (r > _maxradius) {
		return 0.;
	}
	return 1.;
}

static double	sqr(double y) {
	return y * y;
}

//////////////////////////////////////////////////////////////////////
// DiffractionPointSpreadFunction class implementation
//////////////////////////////////////////////////////////////////////

DiffractionPointSpreadFunction::DiffractionPointSpreadFunction(
	const ImageGeometry& geometry) {
	_xfactor =  (M_PI * geometry.aperture())
			/ (geometry.focallength() * 0.000000550);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "_xfactor = %f", _xfactor);
}

double	DiffractionPointSpreadFunction::operator()(double r) const {
	double	x = _xfactor * r;
	double	a = sqr(2 * j1(x) / x);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%f: airy(%f) = %f", r, x, a);
	return a;
}

//////////////////////////////////////////////////////////////////////
// TurbulencePointSpreadFunction class implementation
//////////////////////////////////////////////////////////////////////
TurbulencePointSpreadFunction::TurbulencePointSpreadFunction(double turbulence)
	: _turbulence(turbulence) {
	_norm = 1 / (sqrt(2 * M_PI) * _turbulence);
}

double	TurbulencePointSpreadFunction::operator()(double r) const {
	return _norm * exp(-sqr(r / _turbulence) / 2);
}

} // namespace catalog
} // namespace astro

