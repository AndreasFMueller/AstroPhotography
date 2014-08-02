/*
 * Catalog.cpp -- Catalog access implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCatalog.h>
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <includes.h>

namespace astro {
namespace catalog {

//////////////////////////////////////////////////////////////////////
// SkyWindow
//////////////////////////////////////////////////////////////////////

SkyWindow::SkyWindow(const RaDec& center,
	const Angle& rawidth, const Angle& decheight) : _center(center) {
	_rawidth = rawidth.reduced();
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
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "check whether %s is in %s",
	//	position.toString().c_str(), toString().c_str());
	// check right ascension
	double	left = _center.ra().radians() - _rawidth.radians() / 2.;
	double	right = _center.ra().radians() + _rawidth.radians() / 2.;
	double	ra = reduce(position.ra().radians(), left);
	if (ra > right) {
		return false;
	}
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "check declination");

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

} // namespace catalog
} // namespace astro
