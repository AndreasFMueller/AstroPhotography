/*
 * DitherCalculator.cpp
 *
 * (c) 2019 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroGuiding.h>
#include <AstroDebug.h>
#include <AstroFormat.h>

namespace astro {
namespace guiding {

DitherCalculator::DitherCalculator(const AngularSize& pixelsize)
	: _pixelsize(pixelsize) {
}

Point	DitherCalculator::ditherArcsec(double arcsec) {
	Angle	angle(arcsec / 3600., Angle::Degrees);
	return dither(angle / pixelsize());
}

Point	DitherCalculator::dither(double pixels) {
	// generate vector in polar coordinates
	double	phi = 2 * M_PI * (random() / (double)2147483647);
	double	r = pixels * random() / (double)2147483647;
	Point	v(r * cos(phi), r * sin(phi));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "using dither offset %s",
		v.toString().c_str());
	return v;
}

} // namespace guiding
} // namespace astro
