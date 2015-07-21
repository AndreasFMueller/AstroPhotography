/*
 * SkyWindow.cpp -- SkyWindow implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCatalog.h>
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <includes.h>
#include <AstroIO.h>
#include "CatalogBackend.h"

using namespace astro::io;

namespace astro {
namespace catalog {

SkyWindow::SkyWindow(const RaDec& center,
	const Angle& rawidth, const Angle& decheight) : _center(center) {
	if (rawidth.radians() >= 2 * M_PI) {
		_rawidth.radians(2 * M_PI);
	} else {
		_rawidth = rawidth;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "window dimensions: RA = %f hours, "
		"DEC = %f degrees", rawidth.hours(), decheight.degrees());
	_decheight = decheight.reduced(-M_PI / 2);
}

SkyWindow::SkyWindow(const ImageBase& image) {
	_center.ra().hours(
		(double)(image.getMetadata(std::string("RACENTR"))));
	_center.ra().degrees(
		(double)(image.getMetadata(std::string("DECCENTR"))));
	_rawidth.hours(
		(double)(image.getMetadata(std::string("RAWIDTH"))));
	_decheight.degrees(
		(double)(image.getMetadata(std::string("DECHIGHT"))));
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

void	SkyWindow::addMetadata(ImageBase& image) const {
	image.setMetadata(
		FITSKeywords::meta(std::string("RACENTR"), 
			center().ra().hours()));
	image.setMetadata(
		FITSKeywords::meta(std::string("DECCENTR"), 
			center().dec().degrees()));
	image.setMetadata(
		FITSKeywords::meta(std::string("RAWIDTH"), 
			_rawidth.hours()));
	image.setMetadata(
		FITSKeywords::meta(std::string("DECHIGHT"), 
			_decheight.degrees()));
}

} // namespace catalog
} // namespace astro
