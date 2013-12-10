/*
 * GuiderCalibrator.cpp -- Class to construct calibration data from 
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
 * \brief construct a GuiderCalibrator object
 */
GuiderCalibrator::GuiderCalibrator() {
}

/**
 * \brief add another point to the calibration data
 */
void	GuiderCalibrator::add(double t, const Point& offset,
		const Point& point) {
	calibration_data.push_back(calibration_point(t, offset, point));
}

/**
 * \brief compute the calibration data from the raw points
 */
GuiderCalibration	GuiderCalibrator::calibrate() {
	// build the linear system of equations
	int	m = 2 * calibration_data.size(); // number of equations
	int	n = 8; // number of unknowns
	double	A[n * m];
	double	b[m];

	// fill in equations
	std::vector<calibration_point>::const_iterator	ci;
	int	i = 0;
	for (ci = calibration_data.begin(); ci != calibration_data.end(); ci++){
		A[i        ] = ci->offset.x();
		A[i +     m] = ci->offset.y();
		A[i + 2 * m] = ci->t;
		A[i + 3 * m] = 0;
		A[i + 4 * m] = 0;
		A[i + 5 * m] = 0;
		A[i + 6 * m] = 1;
		A[i + 7 * m] = 0;

		b[i] = ci->point.x();

		i++;

		A[i        ] = 0;
		A[i +     m] = 0;
		A[i + 2 * m] = 0;
		A[i + 3 * m] = ci->offset.x();
		A[i + 4 * m] = ci->offset.y();
		A[i + 5 * m] = ci->t;
		A[i + 6 * m] = 0;
		A[i + 7 * m] = 1;

		b[i] = ci->point.y();

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
	GuiderCalibration	calibration;
	for (unsigned int i = 0; i < 6; i++) {
		calibration.a[i] = b[i];
	}

	// return the calibration data
	return calibration;
}

} // namespace guiding
} // namespace astro
