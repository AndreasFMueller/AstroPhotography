/*
 * SkyRectangle.cpp -- centrally projected Rectangle on the sky
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroChart.h>
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <includes.h>
#include <CatalogBackend.h>

namespace astro {
namespace catalog {

/**
 * \brief SkyRectangle constructor
 *
 * The SkyRectangle implementation uses three unit vectors to compute the
 * central projections. These are a vector pointing to the center of the
 * rectangle, and vectors representing the cartesian coordinate system of
 * the rectangle.
 */
SkyRectangle::SkyRectangle(const SkyWindow& window) : SkyWindow(window) {
	setup();
}

/**
 * \brief Create a default SkyRectangle for the full sky
 */
SkyRectangle::SkyRectangle() : SkyWindow(SkyWindow::all) {
	setup();
}

/**
 * \brief Create a SkyWindow from an image
 */
SkyRectangle::SkyRectangle(const ImageBase& image) : SkyWindow(image) {
	setup();
}

/**
 * \brief Setup the intern
 */
void	SkyRectangle::setup() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create a rectangle");
	direction = UnitVector(this->center());

	// arrow to the right
	RaDec	right;
	right.ra() = this->center().ra() - Angle(M_PI / 2);
	right.dec().degrees(0);
	rightvector = UnitVector(right);

	// arrow up
	upvector = -direction.cross(rightvector);

	// limits
	uplimit = tan(this->decheight() * 0.5);
	rightlimit = tan(this->rawidth() * 0.5);

	// what we did in the constructor
	debug(LOG_DEBUG, DEBUG_LOG, 0, "direction=%s",
		direction.toString().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "right=%s",
		rightvector.toString().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "up=%s",
		upvector.toString().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "uplimit = %f, leftlimit = %f",
		uplimit, rightlimit);
}

/**
 * \brief Find out wheter a point on the sky is projected into the rectangle
 */
bool	SkyRectangle::contains(const RaDec& point) const {
	Point	p = map(point);
	return ((fabs(p.x()) <= rightlimit) && (fabs(p.y()) <= uplimit));
}

/**
 * \brief Map a point on the sky to coordinates relative to the center
 *
 * The coordinate system has the vertical axis pointing to the northern
 * celestial pole. Coordinates of points in the rectangle are in the
 * interval [-1, 1]. Points outside the rectangle can also be mapped provided
 * the are on the half sphere that can be mapped using central projection.
 * Coordinate values can be arbitrarily large in that case.
 */
Point	SkyRectangle::map(const RaDec& where) const {
	// compute the angle between the horizontal plane and the drection
	UnitVector	newpoint(where);
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "checking %s",
	//	newpoint.toString().c_str());

	// newpoint * direction gives the cosine of the angle between the
	// point and the point at the center of the rectangle.
	double	l = newpoint * direction;
	if (l < 0) {
		throw std::runtime_error("cannot image more than half sphere");
	}

	// Scaling the unit vector to the point by the reciprocal of this
	// value gives a vector in the plane of the rectangle. Thus by 
	// projecting the scaled vector onto the vectors rightvector
	// and upvector gives the coordinates in the rectangle
	double	x = (newpoint * rightvector) / l;
	double	y = (newpoint * upvector) / l;

	return Point(x, y);
}

/**
 * \brief Map a point on the sky to the point coordinates in the rectangle
 *
 * The Point value return has coordinates in the interval [0,1], which
 * applications can use to map to pixel coordinates.
 */
Point	SkyRectangle::map2(const RaDec& where) const {
	Point	p = map(where);
	return Point((1 + p.x() / rightlimit) / 2, (1 + p.y() / uplimit) / 2);
}

/**
 * \brief Find a window that contains the image rectangle
 *
 * The SkyRectangle describes a rectangle projected onto the clestial sphere,
 * but for retrieval of stars from the star catalog, a SkyWindow must be
 * specified. This method computes the smallest SkyWindow that contains the
 * projected image rectangle.
 *
 * To compute the window, spherical trigonometry is used.
 */
SkyWindow	SkyRectangle::containedin() const {
	RaDec	newcenter;
	newcenter.ra() = center().ra();
	newcenter.dec() = center().dec();
	Angle	b = rawidth() * 0.5;
	Angle	delta = decheight() * 0.5;
	// north pole contained in image -> lower corners determine radius
	if ((center().dec() + decheight() * 0.5) > Angle(M_PI / 2)) {
		Angle	d = Angle(M_PI / 2) - center().dec();
		Angle	c = d + delta;
		Angle	height((Angle(acos(cos(b) * cos(c))) - d) * 2);
		return SkyWindow(newcenter, Angle(M_PI * 2), height);
	}
	// center in northern hemisphere
	if (center().dec() >= Angle(0)) {
		Angle	c1 = Angle(M_PI / 2) - center().dec() - delta;
		double	a = acos(cos(b) * cos(c1));
		double	B = asin(sin(b) / sin(a));
		Angle	width(2 * B);

		Angle	c2 = Angle(M_PI / 2) - center().dec() + delta;
		a = acos(cos(b) * cos(c2));
		Angle	height = Angle(a) - c1;
		newcenter.dec() = center().dec() + delta - height * 0.5;
		return SkyWindow(newcenter, width, height);
	}
	// south pole contained in image -> upper corners determine radius
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%f",
		(center().dec() - decheight() * 0.5).degrees());
	if ((center().dec() - decheight() * 0.5) < Angle(-M_PI / 2)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "south pole in image");
		Angle	d = Angle(M_PI / 2) + center().dec();
		Angle	c = d + delta;
		Angle	height((Angle(acos(cos(b) * cos(c))) - d) * 2);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "height: %f", height.degrees());
		return SkyWindow(newcenter, Angle(M_PI * 2), height);
	}
	// center in southern hemisphere
	if (center().dec() < Angle(0)) {
		Angle	w = center().dec() * (-1);
		Angle	c1 = Angle(M_PI / 2) - w - delta;
		double	a = acos(cos(b) * cos(c1));
		double	B = asin(sin(b) / sin(a));
		Angle	width(2 * B);

		Angle	c2 = Angle(M_PI / 2) - w + delta;
		a = acos(cos(b) * cos(c2));
		Angle	height = Angle(a) - c1;
		newcenter.dec() = (w + delta - height * 0.5) * (-1);
		return SkyWindow(newcenter, width, height);
	}
	throw std::runtime_error("internal error, center not on sphere");
}

/**
 * \brief Map a point in the image back to the sphere
 *
 * \param p	the point in the image
 */
RaDec	SkyRectangle::inverse(const astro::Point& p) const {
	return RaDec(direction
			+ rightvector * (p.x() * rightlimit)
			+ upvector * (p.y() * uplimit));
}

/**
 * \brief Add the metadata to an image
 */
void	SkyRectangle::addMetadata(ImageBase& image) const {
	SkyWindow::addMetadata(image);
}

} // namespace catalog
} // namespace astro
