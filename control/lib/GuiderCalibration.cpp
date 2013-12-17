/*
 * GuiderCalibration.cpp -- guider calibration
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>

#include <AstroGuiding.h>
#include <AstroFormat.h>
#include <AstroUtils.h>

namespace astro {
namespace guiding {

/**
 * \brief Format the calibration data for display
 */
std::string	GuiderCalibration::toString() const {
	return stringprintf("[ %.3f, %.3f, %.3f; %.3f, %.3f, %.3f ]",
		a[0], a[1], a[2], a[3], a[4], a[5]);
}

/**
 * \brief Construct a new GuiderCalibration object
 */
GuiderCalibration::GuiderCalibration() {
	a[0] = a[3] = 1.;
	a[1] = a[2] = a[4] = a[5] = 0.;
}

/**
 * \brief compute correction for drift
 * 
 * While a correction for some offset depends on the time within which
 * the correction should be done, 
 */
Point	GuiderCalibration::defaultcorrection() const {
	return this->operator()(Point(0, 0), 1);
}

/**
 * \brief Compute correction for an offset
 *
 * The correction to be applied to right ascension and declination depends
 * on the time allotted to the correction. The result is a pair of total
 * corrections. They can either be applied in one second, without any
 * corrections in the remaining seconds of the Deltat-interval, or they can
 * be distributed over the seconds of the Deltat-interval.  This distribution,
 * however, has to be calculated by the caller.
 */
Point	GuiderCalibration::operator()(const Point& offset, double Deltat) const {
	double	Deltax = offset.x() - Deltat * a[2];
	double	Deltay = offset.y() - Deltat * a[5];
        double	determinant = a[0] * a[4] - a[3] * a[1];
        double	x = (Deltax * a[4] - Deltay * a[1]) / determinant;
        double	y = (a[0] * Deltay - a[3] * Deltax) / determinant;
	Point	result(x, y);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "correction for offset %s: %s",
		offset.toString().c_str(), result.toString().c_str());
	return result;
}

/**
 * \brief Access to the calibration data
 */
const double&	GuiderCalibration::operator[](size_t index) const {
	if (index > 5) {
		throw std::range_error("calibration data index too large");
	}
	return a[index];
}

/**
 * \brief Access to the calibration data
 */
double&	GuiderCalibration::operator[](size_t index) {
	if (index > 5) {
		throw std::range_error("calibration data index too large");
	}
	return a[index];
}

/**
 * \brief Rescale the grid dependent part of the calibration
 */
void	GuiderCalibration::rescale(double scalefactor) {
	a[0] *= scalefactor;
	a[1] *= scalefactor;
	a[3] *= scalefactor;
	a[4] *= scalefactor;
}

} // namespace guiding
} // namespace astro
