/*
 * Outline.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroCatalog.h>
#include <AstroDebug.h>
#include <sstream>

namespace astro {
namespace catalog {

std::string	Outline::toString() const {
	std::ostringstream	out;
	out << _name << ":";
	std::for_each(begin(), end(),
		[&](const astro::RaDec& radec) {
			out << " " << radec.toString();
		}
	);
	return out.str();
}

Outline::Outline(const std::string& name, const astro::RaDec& center,
                const astro::TwoAngles& dimensions, const astro::Angle& orientation)
	: _name(name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "construct %s outline", name.c_str());
	
	// create points on an ellipse
	for (float angle = 0; angle < 1.9 * M_PI; angle += M_PI / 6) {
		double	x0 = dimensions.a1().radians() * cos(angle);
		double	y0 = dimensions.a2().radians() * sin(angle);

		// rotate by orientation
		double	x = cos(orientation) * x0 - sin(orientation) * y0;
		double	y = sin(orientation) * x0 + cos(orientation) * y0;

		// compute point on the ellipse
		Angle	r = Angle(hypot(x, y), Angle::Radians);
		Angle	phi = arctan2(y, x);

		// compute spherical triangle
		Angle	a = Angle::right_angle - center.dec();
		Angle	b = arccos(cos(a) * cos(r) + sin(a) * sin(r) * cos(phi));
		Angle	beta = arcsin(sin(r) * sin(phi) / sin(b));

		// compute RA / DEC
		RaDec	radec(center.ra() + beta, Angle::right_angle - b);
		push_back(radec);
	}
}

} // namespace catalog
} // namespace astro
