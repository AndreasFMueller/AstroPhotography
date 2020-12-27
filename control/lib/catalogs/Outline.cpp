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
                 const astro::TwoAngles& axes,
		 const astro::Angle& position_angle)
	: _name(name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "construct %s outline", name.c_str());

	// get the major and minor axes
	double	a = axes.a1().radians() / 2.;
	double	b = axes.a2().radians() / 2.;
	if (a < b) {
		double	temp = a;
		a = b;
		b = temp;
	}

	// if the minor axes is very small, we consider it erroneous and draw
	// a circle instead, i.e. b = a
	if ((b / a) < 0.01) {
		b = a;
	}

	// how many points do we want to create
	const static int	minsteps = 6;
	const static int	maxsteps = 18;
	int	steps = round(maxsteps * a
				/ Angle(0.5, Angle::Degrees).radians());
	if (steps < minsteps) {
		steps = minsteps;
	}
	if (steps > maxsteps) {
		steps = maxsteps;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "drawing ellipse with %d points (%s,%s)",
		steps, Angle(a).dms().c_str(), Angle(b).dms().c_str());
	
	// create points on an ellipse
	float	anglestep = M_PI / steps;
	float	angle = anglestep / 2.;
	for (int i = 0; i < 2 * steps; i++, angle += anglestep) {
		double	x0 = a * cos(angle);
		double	y0 = b * sin(angle);
		Angle	radius(hypot(x0, y0), Angle::Radians);
		Angle	t = position_angle + arctan2(y0, x0);
		RaDec	point = center.exp(t, radius);
		push_back(point);
	}
}

} // namespace catalog
} // namespace astro
