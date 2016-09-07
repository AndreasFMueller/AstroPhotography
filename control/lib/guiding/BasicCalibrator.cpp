/*
 * BasicCalibration.cpp -- Class to construct calibration data from 
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>

#include <AstroGuiding.h>
#include <AstroFormat.h>

#ifdef HAVE_ACCELERATE_ACCELERATE_H
#include <Accelerate/Accelerate.h>
#else
#include <lapack.h>
#endif /* HAVE_ACCELERATE_ACCELERATE_H */

namespace astro {
namespace guiding {

/**
 * \brief construct a BasicCalibration object
 */
BasicCalibrator::BasicCalibrator(const ControlDeviceName& name) {
	_calibration = CalibrationPtr(new BasicCalibration(name));
}

BasicCalibrator::BasicCalibrator(CalibrationPtr calibration) 
	: _calibration(calibration) {
}

/**
 * \brief add another point to the calibration data
 */
void	BasicCalibrator::add(const CalibrationPoint& calibrationpoint) {
	_calibration->add(calibrationpoint);
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
CalibrationPtr	BasicCalibrator::calibrate() {
	// build the linear system of equations
	int	m = 2 * _calibration->size(); // number of equations
	int	n = 8; // number of unknowns
	double	A[n * m];
	double	b[m];

	// fill in equations
	std::vector<CalibrationPoint>::const_iterator	ci;
	int	i = 0;
	for (ci = _calibration->begin(); ci != _calibration->end(); ci++){
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
		_calibration->a[i] = b[i];
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"calibration: [ %.5f, %.5f, %.5f; %.5f, %.5f, %.5f ]",
		_calibration->a[0], _calibration->a[1], _calibration->a[2],
		_calibration->a[3], _calibration->a[4], _calibration->a[5]);

	// The last two variables are not needed for the calibration, we
	// throw them away but it might be interesting to at least note them
	// in the debug log.
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration origin: %.3f, %.3f",
		b[6], b[7]);

	// return the calibration data
	return _calibration;
}

} // namespace guiding
} // namespace astro
