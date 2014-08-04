/*
 * Catalog.cpp -- Catalog access implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCatalog.h>
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <includes.h>
#include <CatalogBackend.h>

namespace astro {
namespace catalog {

//////////////////////////////////////////////////////////////////////
// SkyWindow
//////////////////////////////////////////////////////////////////////

SkyWindow::SkyWindow(const RaDec& center,
	const Angle& rawidth, const Angle& decheight) : _center(center) {
	if (rawidth.radians() >= 2 * M_PI) {
		_rawidth.radians(2 * M_PI);
	} else {
		_rawidth = rawidth;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "window height: %f",
		decheight.degrees());
	_decheight = decheight.reduced(-M_PI / 2);
}

static double	reduce(double x, double left) {
	return x - 2 * M_PI * floor((x - left) / (2 * M_PI));
}

std::string	SkyWindow::toString() const {
	return stringprintf("%.3fx%.3f@%s", _rawidth.hours(),
		_decheight.degrees(), _center.toString().c_str());
}

/**
 * \brief find out whether a position is within the window
 */
bool	SkyWindow::contains(const RaDec& position) const {
	// check right ascension
	if (_rawidth.radians() < (2 * M_PI - 0.000000001)) {
		double	left = _center.ra().radians() - _rawidth.radians() / 2.;
		double	right = _center.ra().radians() + _rawidth.radians() / 2.;
		double	ra = reduce(position.ra().radians(), left);
		if (ra > right) {
			return false;
		}
	}

	// check declination
	double	bottom = _center.dec().radians() - _decheight.radians() / 2.;
	double	top = _center.dec().radians() + _decheight.radians() / 2.;
	double	dec = reduce(position.dec().radians(), bottom);
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "top = %f, bottom = %f, dec = %f",
	//	180 * top / M_PI, 180 * bottom / M_PI, 180 * dec / M_PI);
	if (dec > top) {
		return false;
	}

	// position is inside the window
	return true;
}

std::pair<double, double>	SkyWindow::decinterval() const {
	std::pair<double, double>	result;
	result.first = _center.dec().radians() - _decheight.radians();
	if (result.first < -M_PI/2) {
		result.first = -M_PI/2;
	}
	result.second = _center.dec().radians() + _decheight.radians();
	if (result.second > M_PI/2) {
		result.second = M_PI/2;
	}
	return result;
}

Angle	SkyWindow::leftra() const {
	return Angle(_center.ra() - _rawidth * 0.5).reduced();
}

Angle	SkyWindow::rightra() const {
	return Angle(_center.ra() + _rawidth * 0.5).reduced();
}

Angle	SkyWindow::bottomdec() const {
	return Angle(_center.dec() - _decheight * 0.5).reduced(-M_PI / 2);
}

SkyWindow	SkyWindow::all(RaDec(M_PI, 0), Angle(2 * M_PI), Angle(M_PI));


//////////////////////////////////////////////////////////////////////
// Celestial Object implementation
//////////////////////////////////////////////////////////////////////
/**
 * \brief Compute proper motion corrected position of an object
 */
RaDec	CelestialObject::position(const double epoch) const {
	RaDec	result;
	result.ra() = ra() + pm().ra() * epoch;
	result.dec() = dec() + pm().dec() * epoch;
	return result;
}

//////////////////////////////////////////////////////////////////////
// Star implementation
//////////////////////////////////////////////////////////////////////
/**
 * \brief string representation of a star
 */
std::string	Star::toString() const {
	return stringprintf("%s %s %.2f",
			ra().hms().c_str(), dec().dms().c_str(), mag());
}

//////////////////////////////////////////////////////////////////////
// SkyRectangle implementation
//////////////////////////////////////////////////////////////////////
/**
 * \brief SkyRectangle constructor
 *
 * The SkyRectangle implementation uses three unit vectors to compute the
 * central projections. These are a vector pointing to the center of the
 * rectangle, and vectors representing the cartesian coordinate system of
 * the rectangle.
 */
SkyRectangle::SkyRectangle(const SkyWindow& window) : SkyWindow(window) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create a rectangle");
	direction = UnitVector(window.center());

	// arrow to the right
	RaDec	right;
	right.ra() = window.center().ra() - Angle(M_PI / 2);
	right.dec().degrees(0);
	rightvector = UnitVector(right);

	// arrow up
	upvector = -direction.cross(rightvector);

	// limits
	uplimit = tan(window.decheight() * 0.5);
	rightlimit = tan(window.rawidth() * 0.5);

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
 * \brief 
 */
SkyRectangle::SkyRectangle() : SkyWindow(SkyWindow::all) {
	
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
}

//////////////////////////////////////////////////////////////////////
// Catalog implementation
//////////////////////////////////////////////////////////////////////

/**
 * \brief Create a compilied catalog
 *
 * No catalog is complete enough for our purposes. But getting stars from
 * the files can be quite time consuming. This problem is solved by the
 * database backend, bat that requires quite a bit of disk space.
 * To unify the catalog access and make it transparent for the application,
 * the Catalog constructor decides about the backend to use based on the
 * type of the file argument. If the filename refers to a directory, it
 * is assumed that the file based backend should be used. If filename
 * names a file, it is assumed that this is a database file, and the
 * database backend is used on this file.
 * \param filename	name of database file or base directory for catalogs
 */
Catalog::Catalog(const std::string& filename) {
	// find out whether filename points to a file (i.e. use database
	// backend) or to a directory (use file backend)
	struct stat	sb;
	if (stat(filename.c_str(), &sb) < 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot stat %s: %s",
			filename.c_str(), strerror(errno));
		throw std::runtime_error("cannot stat catalog file");
	}

	// depending on the file type, open different backends
	if (sb.st_mode & S_IFDIR) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "open file backend based at %s",
			filename.c_str());
		backend = CatalogBackendPtr(new FileBackend(filename));
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "open db backend on file %s",
			filename.c_str());
		backend = CatalogBackendPtr(new DatabaseBackend(filename));
	}
}

/**
 * \brief Retrieve stars from a compiled catalog
 */
Catalog::starsetptr	Catalog::find(const SkyWindow& window,
				double minimum_magnitude) {
	return backend->find(window, minimum_magnitude);
}

} // namespace catalog
} // namespace astro
