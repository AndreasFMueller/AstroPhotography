/*
 * ImageCoordinates.cpp -- coordinates of a point in an image
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroCoordinates.h>
#include <cmath>

namespace astro {

/**
 * \brief Construct ImageCoordinates object
 *
 * \param center		image center RA/DEC
 * \param angular_resolution	angular resolution angle/pixel
 * \param azimut		azimuth (rotation of the image)
 * \param mirror		whether or not the image was vertically mirrored
 */
ImageCoordinates::ImageCoordinates(const RaDec& center,
	const Angle& angular_resolution, const Angle& azimut, bool mirror)
	: _center(center), _angular_resolution(angular_resolution),
	  _azimut(azimut), _mirror(mirror) {
}

/**
 * \brief Construct ImageCoordinates object
 *
 * This constructor assumes azimuth angle 0
 *
 * \param center		image center RA/DEC
 * \param angular_resolution	angular resolution angle/pixel
 * \param mirror		whether or not the image was vertically mirrored
 */
ImageCoordinates::ImageCoordinates(const RaDec& center,
	const Angle& angular_resolution, bool mirror)
	: _center(center), _angular_resolution(angular_resolution),
	  _azimut(0), _mirror(mirror) {
}

/**
 * \brief Convert pixel offset to RA and DEC
 *
 * This method converts an offset from the image center to RA and DEC of
 * the point.
 *
 * \param _offset	_offset from image center
 * \returns		RA and DEC of the point
 */
RaDec	ImageCoordinates::offset(const Point& _offset) const {
	double	s = (_mirror) ? -1 : 1;

	// XXX take azimuth into account, we should do this right at the
	// XXX beginning by rotating in the image plane

	// this method uses the following spherical triangle
	//
	//              N beta
	//             / \
	//           a/   \c
	//           /     \
	//   gamma  P-------C alpha
	//              b
	// P = point for which to determine RA/DEC
	// C = center of image
	// N = north pole
	// beta is positive 

	// convert to polar coordinates
	double	radius = hypot(_offset.x(), _offset.y());

	// compute the angle (side) b
	Angle	b = arctan(radius * _angular_resolution.radians());

	// compute the angle alpha
	//
	//   P        |
	//    o       |c        P.x = radius * sin(alpha)
	//     -___   |         P.y = radius * cos(alpha)
	//         ` C    ==> alpha = arctan2(P.x, P.y)
	//       b         

	// compute the angle alpha in 
	Angle	alpha = arctan2(_offset.x(), s * _offset.y());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s -> r = %.2f, alpha = %.1f degrees",
		_offset.toString().c_str(), radius, alpha.degrees());

	// compute spherical triangle, using law of cosines at C to
	// find a
	Angle	c = Angle::right_angle - _center.dec();
	double	cosa = cos(b) * cos(c) + sin(b) * sin(c) * cos(alpha);
	Angle	a = arccos(cosa);

	// use law of sines to compute angle beta
	double	sinbeta = sin(b) * sin(alpha) / sin(a);
	Angle	beta = arcsin(sinbeta);

	// compute new coordinates
	RaDec	result(_center.ra() + beta, Angle::right_angle - a);

	// report findings
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"Delta RA = %.6f[h], Delta DEC = %.6f[deg]",
		result.ra().hours(), result.dec().degrees());
	return result;
}

/**
 * \brief Operator version of the Point to RA/DEC conversion
 *
 * This method just wraps 
 */
RaDec	ImageCoordinates::operator()(const Point& _offset) const {
	return offset(_offset);
}

/**
 * \brief Compute points relative to the center direction
 *
 * This method computes the coordinates in the image of a point
 * at a given RA/DEC.
 *
 * \param direction	RA and DEC of the point
 * \returns		image center 
 */
Point	ImageCoordinates::offset(const RaDec& direction) const {
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
	double	r = tan(b) * _angular_resolution.radians(); // pixels

	// convert polar coordinates into cartesian coordinates
	Point	result(-r * singamma, r * cosgamma);
	return result;
}

/**
 * \brief Operator version of the image coordinate conversion
 */
Point	ImageCoordinates::operator()(const RaDec& direction) const {
	return offset(direction);
}

} // namespace astro
