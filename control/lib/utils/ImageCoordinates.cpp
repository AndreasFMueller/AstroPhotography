/*
 * ImageCoordinates.cpp -- coordinates of a point in an image
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroCoordinates.h>
#include <cmath>

namespace astro {

ImageCoordinates::ImageCoordinates(const RaDec& center,
	const Angle& angular_resolution, const Angle& azimut, bool mirror)
	: _center(center), _angular_resolution(angular_resolution),
	  _azimut(azimut), _mirror(mirror) {
}

ImageCoordinates::ImageCoordinates(const RaDec& center,
	const Angle& angular_resolution, bool mirror)
	: _center(center), _angular_resolution(angular_resolution),
	  _azimut(0), _mirror(mirror) {
}

RaDec	ImageCoordinates::offset(const Point& _offset) const {
	double	s = (_mirror) ? -1 : 1;
	// convert to polar coordinates
	// XXX there probably is a bug here because right ascension
	// XXX increases to the right
	double	radius = hypot(_offset.x(), s * _offset.y());
	Angle	phi(atan2(s * _offset.y(), _offset.x()));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s -> r = %.2f, phi = %.1f degrees",
		_offset.toString().c_str(), radius, phi.degrees());

	// compute spherical triangle
	Angle	a = Angle::right_angle - _center.dec();
	Angle	gamma = Angle::right_angle - (phi + _azimut);
	Angle	b = radius * _angular_resolution;
	
	Angle	c = arccos(cos(a) * cos(b) + sin(a) * sin(b) * cos(gamma));
	Angle	d = a - c;

	Angle	beta(arcsin(sin(b) * sin(gamma) / sin(c)));

	debug(LOG_DEBUG, DEBUG_LOG, 0, "a = %.4f[deg], b = %.4f[deg], "
		"c = %.4f[deg], gamma = %.4f[deg], beta = %.4f[deg]",
		a.degrees(), b.degrees(), c.degrees(),
		gamma.degrees(), beta.degrees());

	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"Delta RA = %.6f[h], Delta DEC = %.6f[deg]",
		beta.hours(), d.degrees());

	// extract RA and DEC
	RaDec	result(beta, d);
	return result;
}

RaDec	ImageCoordinates::operator()(const Point& _offset) const {
	return _center + offset(_offset);
}

/**
 * \brief Compute points relative to the center direction
 */
Point	ImageCoordinates::operator()(const RaDec& direction) const {
	// we use a spherical triangle from the north pole (B)
	// to the center (C) to the direction (A):
	//
	//          B
	//         / \
	//      a /   \ c
	//       /     \
	//      C-------A
	//          b
	// The angle gamma is at the point C
	// a is a right angle - declination of C
	// c is a right angle - declination of A
	// beta (the angle at B) is the difference of right ascensions
	//      of A and C
	// this allows to determine b with the law of cosines
	Angle	a = Angle::right_angle - _center.dec();
	Angle	c = Angle::right_angle - direction.dec();
	Angle	beta = direction.ra() - _center.ra();
	double	cosb = cos(c) * cos(a) + sin(c) * sin(a) * cos(beta);
	Angle	b = arccos(cosb);

	// sin(gamma) can be determined with the law of sines
	double	singamma = sin(c) * sin(beta) / sin(b);

	// cos(gamma9 can be determined with the law of cosines
	double	cosgamma = (cos(c) - cos(a) * cos(b)) / (sin(a) * sin(b));

	// determine the radius
	double	r = b.radians() / _angular_resolution.radians(); // pixels

	// convert polar coordinates into cartesian coordinates
	Point	result(-r * singamma, r * cosgamma);
	return result;
}

} // namespace astro
