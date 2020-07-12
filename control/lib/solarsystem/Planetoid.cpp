/*
 * Planetoid.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroSolarsystem.h>
#include <sstream>
#include <iomanip>
#include <AstroFormat.h>

namespace astro {
namespace solarsystem {

/**
 * \brief Construct a planetoid
 *
 * \param name		name of the planetoid
 * \param a		semimajor axis
 * \param e		excentricity
 * \param Omega		ascending node
 * \param i		inclination
 * \param omega		perihelion length
 * \param n		speed
 * \param M0		mean anomaly at time 0
 */
Planetoid::Planetoid(const std::string& name,
		double a, double e, const Angle& Omega, const Angle& i,
                const Angle& omega, const Angle& n, const Angle& M0)
	: _name(name), _a(a), _e(e), _Omega(Omega), _i(i), _omega(omega),
	  _n(n), _M0(M0) {
}

inline static double	sqr(const double x) {
	return x * x;
}

/**
 * \brief Compute the ecliptical longitude of the planet
 *
 * \param m	time from 2000.0 equinox in julian centuries 
 */
Angle   Planetoid::l(const SinCos& m) const {
	Angle	result = _omega + m;
	SinCos	m2 = m * 2;
	result = result + Angle(2 * _e * m.sin());
	result = result + Angle(
	    (1.25 * sqr(_e) - sqr(tan(0.5 * _i)) * cos(2 * _omega)) * m2.sin()
	);
	result = result + Angle(-sqr(tan(0.5 * _i)) * sin(2*_omega) * m2.cos());
	return result;
}

/**
 * \brief Compute the ecliptical latitude of the planet
 *
 * \param m	time from 2000.0 equinox in julian centuries 
 */
Angle   Planetoid::b(const SinCos& m) const {
	SinCos	o(_omega);
	Angle	result = -_i * _e * o.sin();
	result = result + _i * (o.sin() * m.cos() + o.cos() * m.sin());
	SinCos	M = m * 2;
	result = result + _i * _e * (o.sin() * M.cos() + o.cos() * M.sin());
	return result;
}

/**
 * \brief Compute the distance to the sun
 *
 * \param m	time from 2000.0 equinox in julian centuries
 */
double  Planetoid::r(const SinCos& m) const {
	return _a * (1 + sqr(_e) / 2.)
		- _a * _e * m.cos()
		- (_a * sqr(_e) / 2.) * (m*2).cos();
}

/**
 * \brief Compute the mean anomaly of the planetoid
 *
 * \param T	time from 2000.0 equinox in julian centuries
 */
Angle   Planetoid::M(const JulianCenturies& T) const {
	return _M0 + _n * T;
}

/**
 * \brief Compute sin and cosine of the mean anomaly of the planetoid
 *
 * \param T	time from 2000.0 equinox in julian centuries
 */
SinCos	Planetoid::Msc(const JulianCenturies& T) const {
	return SinCos(M(T));
}

/**
 * \brief Compute ecliptical coordinates 
 *
 * \param T	time from 2000.0 equinox in julian centuries
 */
EclipticalCoordinates   Planetoid::ecliptical(const JulianCenturies& T) const {
	SinCos	m = Msc(T);
	return EclipticalCoordinates(l(m), r(m), b(m));
}

/**
 * \brief Format the elements as a string
 */
std::string	Planetoid::toString(Angle::unit u) const {
	std::ostringstream	out;
	out << name() << ":" << std::endl;

	out << stringprintf("a =     %11.6f  ", a());
	out << stringprintf("e =  %9.6f  ", e());
	out << stringprintf("M0 =  %11.6f  ", M0().value(u));
	out << stringprintf("n =  %12.4f  ", n().value(u));

	out << std::endl;

	out << stringprintf("Omega = %11.6f  ", Omega().value(u));
	out << stringprintf("i =%9.4f  ", i().value(u));
	out << stringprintf("omega = %11.6f  ", omega().value(u));

	out << std::endl;
	return out.str();
}

/**
 * \brief Display elements of a planetoid
 *
 * \param out		the output stream to write the information to
 * \param planetoid	the planetoid with the elements to display
 */
std::ostream&	operator<<(std::ostream& out, const Planetoid& planetoid) {
	out << planetoid.toString();
	return out;
}

} // namespace solarsystem
} // namespace astro
