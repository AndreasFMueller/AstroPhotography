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
#ifdef HAVE_ACCELERATE_ACCELERATE_H
#include <Accelerate/Accelerate.h>
#else
#include <lapack.h>
#endif /* HAVE_ACCELERATE_ACCELERATE_H */

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
	_masPerPixel = 0;
	_focallength = 0;
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
	_masPerPixel = 0;
	_focallength = 0;
	time(&_when);
}

/**
 * \brief Construct from another basic calibration
 */
BasicCalibration::BasicCalibration(const BasicCalibration& other)
	: _name(other.name()) {
	_calibrationid = -1;
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
	_masPerPixel = other._masPerPixel;
	_focallength = other._focallength;

	// copy points
	clear();
	std::copy(other.begin(), other.end(), back_inserter(*this));
}

/**
 * \brief compute the calibration data from the raw points
 *
 * The guider port activations move a star over the ccd area. The velocity
 * of this movement is measure in pixels/second. The vector of movement 
 * induced by the activation of the right ascension guider port controls
 * has components vx_ra and vy_ra, they are unknowns 0 and 3. The velocity
 * induced by declination port activation has components vx_dec and vy_dec,
 * they are unknowns 1 and 4. The drift velocity describes the movement of
 * the star without any controls applied, they are drift_x and drift_y,
 * unknowns 2 and 5. The remaining two unknowns 6 and 7 are origin_x and
 * origin_y, they are the best estimate of the origin at the beginning of the
 * calibration process (time origin).
 */
void	BasicCalibration::calibrate() {
	// build the linear system of equations
	int	m = 2 * size(); // number of equations
	int	n = 8; // number of unknowns
	double	A[n * m];
	double	b[m];

	// fill in equations
	std::vector<CalibrationPoint>::const_iterator	ci;
	int	i = 0;
	for (ci = begin(); ci != end(); ci++){
		A[i        ] = ci->offset.x();	// vx_ra
		A[i +     m] = ci->offset.y();	// vx_dec
		A[i + 2 * m] = ci->t;		// drift_x
		A[i + 3 * m] = 0;		// vy_ra
		A[i + 4 * m] = 0;		// vy_dec
		A[i + 5 * m] = 0;		// drift_y
		A[i + 6 * m] = 1;		// origin_x
		A[i + 7 * m] = 0;		// origin_y

		b[i] = ci->star.x();

		i++;

		A[i        ] = 0;
		A[i +     m] = 0;
		A[i + 2 * m] = 0;
		A[i + 3 * m] = ci->offset.x();
		A[i + 4 * m] = ci->offset.y();
		A[i + 5 * m] = ci->t;
		A[i + 6 * m] = 0;
		A[i + 7 * m] = 1;

		b[i] = ci->star.y();

		i++;
	}

	// prepare to solve the system using LAPACK (dgels_)
	char	trans = 'N';
	int	nrhs = 1;
	int	lda = m;
	int	ldb = m;
	int	lwork = -1;
	int	info = 0;

	// determine work area size
	double	x;
	dgels_(&trans, &m ,&n, &nrhs, A, &lda, b, &ldb, &x, &lwork, &info);
	if (info != 0) {
		std::string	msg = stringprintf("dgels cannot determine "
			"work area size: %d", info);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	lwork = x;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "need work area of size %d", lwork);

	//  allocate work array
	double	work[lwork];
	dgels_(&trans, &m ,&n, &nrhs, A, &lda, b, &ldb, work, &lwork, &info);
	if (info != 0) {
		std::string	msg = stringprintf("dgels cannot solve "
			"equations: %d", info);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// store the results in the calibration data array
	for (unsigned int i = 0; i < 6; i++) {
		a[i] = b[i];
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"calibration: [ %.5f, %.5f, %.5f; %.5f, %.5f, %.5f ]",
		a[0], a[1], a[2],
		a[3], a[4], a[5]);

	// The last two variables are not needed for the calibration, we
	// throw them away but it might be interesting to at least note them
	// in the debug log.
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration origin: %.3f, %.3f",
		b[6], b[7]);

	// the calibration is now complete
	_complete = true;
}

} // namespace guiding
} // namespace astro

