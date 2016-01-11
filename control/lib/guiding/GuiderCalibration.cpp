/*
 * GuiderCalibration.cpp -- guider calibration
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>

#include <AstroGuiding.h>
#include <AstroFormat.h>
#include <AstroUtils.h>
#include <sstream>

namespace astro {
namespace guiding {

/**
 * \brief convert a Calibration Point to a string
 */
std::ostream&	operator<<(std::ostream& out, const CalibrationPoint& cal) {
	out << cal.t << "," << cal.offset << "," << cal.star;
	return out;
}

/**
 * \brief Format the calibration data for display
 */
std::string	GuiderCalibration::toString() const {
	std::ostringstream	out;
	out << *this;
	return out.str();
}

/**
 * \brief Construct a new GuiderCalibration object
 *
 * The default calibration has all members set to zero, in particular,
 * it cannot be inverted, and it is not possible to compute corrections.
 */
GuiderCalibration::GuiderCalibration() {
	a[0] = a[1] = a[2] = a[3] = a[4] = a[5] = 0.;
	complete = false;
	focallength = 0;
}

/**
 * \brief Determinant of the calibration
 */
double	GuiderCalibration::det() const {
	return a[0] * a[4] - a[1] * a[3];
}

/**
 * \brief Construct a GuiderCalibration object from coefficient array
 */
GuiderCalibration::GuiderCalibration(const double coefficients[6]) {
	for (int i = 0; i < 6; i++) {
		a[i] = coefficients[i];
	}
	complete = false;
	focallength = 0;
}

/**
 * \brief Construct a GuiderCalibration object from coefficient array
 */
GuiderCalibration::GuiderCalibration(const float coefficients[6]) {
	for (int i = 0; i < 6; i++) {
		a[i] = coefficients[i];
	}
	complete = false;
	focallength = 0;
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
        double	determinant = det();
	if (0 == det()) {
		throw std::runtime_error("no calibration");
	}
	double	Deltax = offset.x() - Deltat * a[2];
	double	Deltay = offset.y() - Deltat * a[5];
        double	x = (Deltax * a[4] - Deltay * a[1]) / determinant;
        double	y = (a[0] * Deltay - a[3] * Deltax) / determinant;
	Point	result(x, y);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "correction for offset %s: %s",
		offset.toString().c_str(), result.toString().c_str());
	return result;
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

/**
 * \brief Output of guider calibration data
 */
std::ostream&	operator<<(std::ostream& out, const GuiderCalibration& cal) {
	out << "[" << cal[0] << "," << cal[1] << "," << cal[2] << ";";
	out << cal[3] << "," << cal[4] << "," << cal[5] << "]";
	return out;
}

/**
 * \brief Parse a guider calibration
 */
std::istream&	operator>>(std::istream& in, GuiderCalibration& cal) {
	double	a[6];
	absorb(in, '[');
	in >> a[0];
	absorb(in, ',');
	in >> a[1];
	absorb(in, ',');
	in >> a[2];
	absorb(in, ';');
	in >> a[3];
	absorb(in, ',');
	in >> a[4];
	absorb(in, ',');
	in >> a[5];
	absorb(in, ']');
	// only if we get to this point can we assume that the calibration
	// was successfully read, and can copy it to the target calibration
	for (int i = 0; i < 6; i++) { cal.a[i] = a[i]; }
	return in;
}

/*
 * \brief Compute guider quality figure of merit
 *
 */
double	GuiderCalibration::quality() const {
	double	l1 = hypot(a[0], a[3]);
	double	l2 = hypot(a[1], a[4]);
	double	cosalpha = (a[0] * a[1] + a[3] * a[4]) / (l1 * l2);
	return 1 - (cosalpha * cosalpha);
}

} // namespace guiding
} // namespace astro
