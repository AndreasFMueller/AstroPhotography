/*
 * Planetoid.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroSolarsystem.h>

namespace astro {
namespace solarsystem {

Planetoid::Planetoid(const std::string& name,
		double a, double e, const Angle& Omega, const Angle& i,
                const Angle& omega, const Angle& omegabar, const Angle& n,
		const Angle& M0)
	: _name(name), _a(a), _e(e), _Omega(Omega), _i(i), _omega(omega),
	  _omegabar(omegabar), _n(n), _M0(M0) {
}

inline static double	sqr(const double x) {
	return x * x;
}

Angle   Planetoid::l(const JulianCenturies& T) const {
	Angle	m = M(T);
	Angle	result = _omegabar + m;
	result = result + Angle(2 * _e * sin(m));
	result = result + Angle(
	    (1.25 * sqr(_e) - sqr(tan(0.5 * _i)) * cos(2 * _omega)) * sin(2*m)
	);
	result = result + Angle(-sqr(tan(0.5 * _i)) * sin(2*_omega) * cos(2*m));
	return result;
}

Angle   Planetoid::b(const JulianCenturies& T) const {
	Angle	m = M(T);
	SinCos	_M(m);
	SinCos	o(_omega);
	Angle	result = -_i * _e * o.sin();
	result = result + _i * (o.sin() * _M.cos() + o.cos() * _M.sin());
	_M = _M * 2;
	result = result + _i * _e * (o.sin() * _M.cos() + o.cos() * _M.sin());
	return result;
}

double  Planetoid::r(const JulianCenturies& T) const {
	Angle	m = M(T);
	return _a * (1 + sqr(_e) / 2.)
		- _a * _e * cos(m)
		- (_a * sqr(_e) / 2.) * cos(2 * m);
}

Angle   Planetoid::M(const JulianCenturies& T) const {
	return _M0 + _n * T;
}

SinCos	Planetoid::Msc(const JulianCenturies& T) const {
	return SinCos(M(T));
}

EclipticalCoordinates   Planetoid::ecliptical(const JulianCenturies& T) const {
	return EclipticalCoordinates(l(T), r(T), b(T));
}

} // namespace solarsystem
} // namespace astro
