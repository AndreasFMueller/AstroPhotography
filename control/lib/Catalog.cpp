/*
 * Catalog.cpp -- Catalog access implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCatalog.h>
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <includes.h>
#include <DatabaseCatalog.h>

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
RaDec	CelestialObject::position(const double epoch) const {
	RaDec	result;
	result.ra() = ra() + pm().ra() * epoch;
	result.dec() = dec() + pm().dec() * epoch;
	return result;
}

//////////////////////////////////////////////////////////////////////
// Star implementation
//////////////////////////////////////////////////////////////////////
std::string	Star::toString() const {
	return stringprintf("%s %s %.2f",
			ra().hms().c_str(), dec().dms().c_str(), mag());
}

//////////////////////////////////////////////////////////////////////
// SkyRectangle implementation
//////////////////////////////////////////////////////////////////////
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

SkyRectangle::SkyRectangle() : SkyWindow(SkyWindow::all) {
	
}

bool	SkyRectangle::contains(const RaDec& point) const {
	Point	p = map(point);
	return ((fabs(p.x()) <= rightlimit) && (fabs(p.y()) <= uplimit));
}

Point	SkyRectangle::map(const RaDec& where) const {
	// compute the angle between the horizontal plane and the drection
	UnitVector	newpoint(where);
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "checking %s",
	//	newpoint.toString().c_str());
	double	l = newpoint * direction;
	if (l < 0) {
		throw std::runtime_error("cannot image more than half sphere");
	}
	double	x = (newpoint * rightvector) / l;
	double	y = (newpoint * upvector) / l;
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "x = %f, y = %f", x, y);
	return Point(x, y);
}

Point	SkyRectangle::map2(const RaDec& where) const {
	Point	p = map(where);
	return Point((1 + p.x() / rightlimit) / 2, (1 + p.y() / uplimit) / 2);
}

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

Catalog::Catalog(const std::string& filename) {
	database = DatabaseCatalogPtr(new DatabaseCatalog(filename));
}

Catalog::starsetptr	Catalog::find(const SkyWindow& window,
				double minimum_magnitude) {
	return database->find(window, minimum_magnitude);
}

} // namespace catalog
} // namespace astro
