/*
 * BasicCalibration.cpp -- common calibration base class implementation
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
std::string	BasicCalibration::toString() const {
	std::ostringstream	out;
	out << *this;
	return out.str();
}

/**
 * \brief Construct a new BasicCalibration object
 *
 * The default calibration has all members set to zero, in particular,
 * it cannot be inverted, and it is not possible to compute corrections.
 */
BasicCalibration::BasicCalibration(const ControlDeviceName& name)
	: _name(name) {
	_calibrationid = -1;
	a[0] = a[1] = a[2] = a[3] = a[4] = a[5] = 0.;
	_complete = false;
	_flipped = false;
	time(&_when);
}

/**
 * \brief Determinant of the calibration
 *
 * The determinant is the area of the first two column vectors of the a-matrix.
 * If this area is small, then a large control activation only leads to
 * small changes, which makes it impossible to compute good corrections.
 * The inverse matrix will have large entries in this case. Ideally, the
 * determinant should be around 1.
 */
double	BasicCalibration::det() const {
	return a[0] * a[4] - a[1] * a[3];
}

/**
 * \brief Construct a BasicCalibration object from the coefficient array
 *
 * This also sets the complete value, because we want that this 
 * calibration becomes usable after setting the coefficients.
 */
BasicCalibration::BasicCalibration(const ControlDeviceName& name,
	const double coefficients[6]) : _name(name) {
	_calibrationid = -1;
	for (int i = 0; i < 6; i++) {
		a[i] = coefficients[i];
	}
	_complete = true;
	_flipped = false;
	time(&_when);
}

/**
 * \brief Construct from another basic calibration
 */
BasicCalibration::BasicCalibration(const BasicCalibration& other)
	: _name(other.name()) {
	copy(other);
}

/**
 * \brief compute correction for drift
 * 
 * While a correction for some offset depends on the time within which
 * the correction should be done, 
 */
Point	BasicCalibration::defaultcorrection() const {
	return this->correction(Point(0, 0), 1);
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
Point	BasicCalibration::correction(const Point& offset, double Deltat) const {
        double	determinant = det();
	if (0 == det()) {
		throw std::runtime_error("no calibration");
	}
	double	s = (flipped()) ? -1 : 1;
	double	Deltax = -(s * offset.x()) - Deltat * a[2];
	double	Deltay = -(s * offset.y()) - Deltat * a[5];
        double	x = ( a[4] * Deltax - a[1] * Deltay) / determinant;
        double	y = (-a[3] * Deltax + a[0] * Deltay) / determinant;
	Point	result(x, y);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "correction for offset %s: %s",
		offset.toString().c_str(), result.toString().c_str());
	return result;
}

/**
 * \brief Compute the pixel offset that we would see after a correction
 */
Point	BasicCalibration::offset(const Point& point, double Deltat) const {
	double	s = (flipped()) ? -1 : 1;
	double	x = s * (a[0] * point.x() + a[1] * point.y() + a[2] * Deltat);
	double	y = s * (a[3] * point.x() + a[4] * point.y() + a[5] * Deltat);
	Point	result(x, y);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "offset %s for correction %s, t=%.1f",
		result.toString().c_str(), point.toString().c_str(), Deltat);
	return result;
}

/**
 * \brief Rescale the grid dependent part of the calibration
 */
void	BasicCalibration::rescale(double scalefactor) {
	a[0] *= scalefactor;
	a[1] *= scalefactor;
	a[3] *= scalefactor;
	a[4] *= scalefactor;
}

/**
 * \brief Output of guider calibration data
 */
std::ostream&	operator<<(std::ostream& out, const BasicCalibration& cal) {
	double	s = (cal.flipped()) ? -1 : 1;
	out << "[" << (s * cal.a[0]) << "," << (s * cal.a[1])
			<< "," << cal.a[2] << ";";
	out <<        (s * cal.a[3]) << "," << (s * cal.a[4])
			<< "," << cal.a[5] << "]";
	return out;
}

/**
 * \brief Parse a guider calibration
 */
std::istream&	operator>>(std::istream& in, BasicCalibration& cal) {
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
 * This quality measure is essentially the sin^2 of the angle between
 * the to two vectors of the a-matrix. If the vectors are orthogonal,
 * then this number will be close to 1, but will never exceed it. If
 * the vectors are too short, then the angle will be more or less random,
 * and the quality will often be low. In these cases, the determinant
 * should also be used, which will then be too small.
 */
double	BasicCalibration::quality() const {
	double	l1 = hypot(a[0], a[3]);
	double	l2 = hypot(a[1], a[4]);
	double	cosalpha = (a[0] * a[1] + a[3] * a[4]) / (l1 * l2);
	double	result =  1 - (cosalpha * cosalpha);
	if (result != result) {
		result = 0;
	}
	return result;
}

void	BasicCalibration::reset() {
	_calibrationid = 0;
	calibrationtype(GP);
	for (int i = 0; i < 6; i++) { a[i] = 0; }
	_complete = false;
	clear();
}

BasicCalibration&	BasicCalibration::operator=(const BasicCalibration& other) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "copying basic calibration");
	copy(other);
	return *this;
}

void	BasicCalibration::copy(const BasicCalibration& other) {
	// carefully copy calibration id, don't overwrite an id if it
	// is already > 0
	if (_calibrationid <= 0) {
		_calibrationid = other.calibrationid();
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibrationid = %d", _calibrationid);

	// copy common fields
	_name = other._name;
	_when = other._when;
	for (int i = 0; i < 6; i++) { a[i] = other.a[i]; }
	_complete = other._complete;
	_flipped = other._flipped;

	// copy points
	clear();
	BasicCalibration	*bc = this;
	for_each(begin(), end(), [bc](CalibrationPoint point) {
			bc->add(point);
		}
	);
}

} // namespace guiding
} // namespace astro
