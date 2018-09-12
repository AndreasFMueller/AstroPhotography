/*
 * GridConstant.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <GridConstant.h>
#include <math.h>
#include <AstroDebug.h>
#include <AstroFormat.h>

namespace astro {
namespace guiding {

static const double	siderial_rate = 2 * M_PI / 86400;
static const double	arcsec_per_radian = 180 * 3600 / M_PI;

/**
 * \brief Get angular size of a pixel
 *
 * \return angle in radians in one pixel
 */
double	GridConstant::angle_per_pixel() const {
	return _pixelsize / _focallength;
}

/**
 * \brief Get angle moved per second at current guiderate
 *
 * \return angle in radians moved in one second
 */
double	GridConstant::angle_per_second() const {
	return siderial_rate * _guiderate;
}

/**
 * \brief get Pixels per radian
 *
 * \return number of pixels in one radian
 */
double	GridConstant::pixels_per_angle() const {
	return 1 / angle_per_pixel();
}

/**
 * \brief Get Pixels moved per second
 *
 * \return number of pixels moved in one second
 */
double	GridConstant::pixels_per_second() const {
	return pixels_per_angle() * angle_per_second();
}

/**
 * \brief Get angular size in arc seconds
 */
double	GridConstant::arcsec_per_pixel() const {
	return arcsec_per_radian * angle_per_pixel();
}

/**
 * \brief Get angular movement in arc seconds in one second
 */
double	GridConstant::arcsec_per_second() const {
	return arcsec_per_pixel() * pixels_per_second();
}

/**
 * \brief Construct a GridConstant object
 *
 * \param focallength	focal length in meters
 * \param pixelsize	size of a pixel in meters
 */
GridConstant::GridConstant(double focallength, double pixelsize)
	: _focallength(focallength), _pixelsize(pixelsize), _guiderate(0.5) {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"GridConstnat(focallength = %.3fmm, pixelsize = %.1fum",
		_focallength, _pixelsize);

	// make sure we have a reasonable focal length
	if (_focallength < 0) {
		std::string	msg = stringprintf("focal length %.3f cannot "
			"be negative", _focallength);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	if (_focallength == 0) {
		_focallength = 0.24;
		debug(LOG_WARNING, DEBUG_LOG, 0,
			"focal length undefined, using %.3f", _focallength);
	}
	if (_focallength > 100) {
		std::string	msg = stringprintf("focal length %f too large",
			_focallength);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// make sure we have a reasonable pixel size
	if ((_pixelsize <= 0) || (_pixelsize > 0.000100)) {
		std::string	msg = stringprintf("pixel size %.1f must "
			"be <= 100um and positive", _pixelsize * 1e6);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
}

/**
 * \brief Get the time for a given movemend in arcseconds
 *
 * \param arcseconds	required angular movement in arcseconds
 * \return		number of seconds
 */
double	GridConstant::suggested_arcsec(double arcseconds) const {
	return arcseconds / arcsec_per_second();
}

/**
 * \brief Get the time for a given movemend in pixels
 *
 * \param pixels	required angular movement in pixels
 * \return		number of seconds
 */
double	GridConstant::suggested_pixel(double pixels) const {
	return pixels / pixels_per_second();
}

/**
 * \brief Get the time for a given movemend in arcseconds
 *
 * This is the method normally used, it clamps the time returned to 
 * a reasonable interval by first clamping the number of pixels to
 * a reasonable interval.
 *
 * \param pixels	required movement in pixels
 * \return		number of seconds
 */
double	GridConstant::operator()(double pixels) const {
	// make sure angle is large enough, at least 5 arc seconds, because
	// smaller angles are mechanically not reasonable
	double	arcseconds = pixels * arcsec_per_pixel();
	if (arcseconds < 5) {
		pixels = 5 / arcsec_per_pixel();
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"angle %f[arcsec] too small, use 5[arcsec] = %.1f[px]",
			arcseconds, pixels);
	}

	// make sure the number of pixels displacement is reasonable
	if (pixels > 100) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"pixel displacement %.0f[px] too large, using 100",
			pixels);
		pixels = 100;
	}
	if (pixels < 5) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"to few pixels %.0f displacement, using 5",
			pixels);
		pixels = 5;
	}
	double	seconds = pixels / pixels_per_second();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "using %.1f[s], %.1[arcsec], %.1[px]",
		seconds, pixels, pixels * arcsec_per_pixel());
	return seconds;
}

/**
 * \brief Compute the pixel size from focal length and angle
 *
 * \param focallength	focal length of the guider in [m]
 * \param arcsec	angular size of pixel in arc seconds
 */
double	GridConstant::pixelsize_from_arcsec(double focallength, double arcsec) {
	return pixelsize_from_angle(focallength, arcsec / arcsec_per_radian);
}

/**
 * \brief Compute the pixelsize from an angle in radians
 *
 * \param focallength	focal length of the guider in [m]
 * \param angle		angular size of pixel in radians
 */
double	GridConstant::pixelsize_from_angle(double focallength, double angle) {
	return focallength * angle;
}

} // namespace guiding
} // namespace astro
