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

/**
 * \brief Construct a sky window based on a angles given as arguments
 *
 * \param center	center of the sky window
 * \param rawidth	width of the window
 * \param decheight	height of the window
 */
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

/**
 * \brief Construct SkyWindow based on the data found in an image
 *
 * \param image		image to read window dimensions from metadata
 */
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

/**
 * \brief Construct a SkyWindow that contains the complete sky
 */
SkyWindow::SkyWindow() {
	_rawidth.radians(4 * M_PI);
	_decheight.radians(2 * M_PI);
}

static double	reduce(double x, double left) {
	return x - 2 * M_PI * floor((x - left) / (2 * M_PI));
}

std::string	SkyWindow::toString() const {
	return stringprintf("%.3f[h]x%.3f[deg]@%s", _rawidth.hours(),
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

Angle	SkyWindow::topdec() const {
	return Angle(_center.dec() + _decheight * 0.5);
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

typedef struct abeta_s {
	Angle	a;
	Angle	beta;
} abeta_t;

/**
 * \brief Compute hypothenuse and angle at top of right triangle 
 *
 * \param b 	base of the triangle
 * \param c	height of the triangle
 */
static abeta_t	a(const Angle& b, const Angle& c) {
	abeta_t	result;
	double	cosa = cos(b) * cos(c);
	if (cosa >  1) { cosa =  1; }
	if (cosa < -1) { cosa = -1; }
	result.a = arccos(cosa);
	result.beta = arccos((cos(b) - cos(result.a) * cos(c))
				/ (sin(result.a) * sin(c)));
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"a = %.2f, b = %.2f, c = %.2f, beta = %.2f",
		result.a.degrees(), b.degrees(), c.degrees(),
		result.beta.degrees());
	return result;
}

/**
 * \brief Construct a window that contains a rectangle
 *
 * \param center	center of the rectangle
 * \param rawidth	width of the rectangle
 * \param decheight	height of the rectangle
 */
SkyWindow	SkyWindow::hull(const RaDec& center, const Angle& rawidth,
			const Angle& decheight) {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"hull for center=%s, width=%.1f, height=%.1f",
		center.toString().c_str(), rawidth.degrees(),
		decheight.degrees());

	Angle	top = center.dec() + 0.5 * decheight;
	Angle	dectop;
	Angle	newrawidthtop;
	if (top >= Angle::right_angle) {
		dectop = Angle::right_angle;
		newrawidthtop = Angle(2 * M_PI);
	} else {
		abeta_t	bb = a(0.5 * rawidth, Angle::right_angle - top);
		newrawidthtop = 2 * bb.beta;
		if (top >= Angle(0)) {
			dectop = top;
		} else {
			dectop = Angle::right_angle - bb.a;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "top=%.1f, newrawidthtop=%.1f",
		dectop.degrees(), newrawidthtop.degrees());

	Angle	bottom = center.dec() - 0.5 * decheight;
	Angle	decbottom;
	Angle	newrawidthbottom;
	if (bottom <= -Angle::right_angle) {
		decbottom = -Angle::right_angle;
		newrawidthbottom = Angle(2 * M_PI);
	} else {
		abeta_t	bb = a(0.5 * rawidth, Angle::right_angle - bottom);
		newrawidthbottom = 2 * bb.beta;
		if (bottom < Angle(0)) {
			decbottom = bottom;
		} else {
			decbottom = Angle::right_angle - bb.a;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "bottom=%.1f, newrawidthbottom=%.1f",
		decbottom.degrees(), newrawidthbottom.degrees());

	Angle	deccenter = 0.5 * (dectop + decbottom);
	RaDec	newcenter(center.ra(), deccenter);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new center: %s",
		newcenter.toString().c_str());

	SkyWindow	result(newcenter,
				std::max(newrawidthtop, newrawidthbottom),
				dectop - decbottom);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "hull sky window: %s",
		result.toString().c_str());
	return result;
}

} // namespace catalog
} // namespace astro
