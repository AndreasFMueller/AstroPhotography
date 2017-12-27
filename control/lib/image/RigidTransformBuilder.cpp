/*
 * RigidTransformBuilder.cpp
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "TransformBuilder.h"

#ifdef HAVE_ACCELERATE_ACCELERATE_H
#include <Accelerate/Accelerate.h>
#else
#include <lapack.h>
#endif /* HAVE_ACCELERATE_ACCELERATE_H */

namespace astro {
namespace image {
namespace transform {

/**
 * \brief common method to solve least squares equations
 */
Transform	RigidTransformBuilder::build(const std::vector<Point>& from,
		const std::vector<Point>& to,
		const std::vector<double>& weights) {
	if (from.size() != to.size()) {
		std::string	msg = stringprintf("point vector size mismatch:"
			" %d != %d", from.size(), to.size());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	bool	ignore_weights = (from.size() != weights.size());

	// allocate space for the linear system
	int	m = 2 * from.size();
	double	A[4 * m];
	double	b[m];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "A size: %d, b size: %d", 4 * m, m);

	// set up linear system of equations
	int	i = 0;
	auto	fromptr = from.begin();
	auto	toptr = to.begin();
	auto	weightptr = weights.begin();
	while (fromptr != from.end()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s ~ %s, delta = %s",
			fromptr->toString().c_str(),
			toptr->toString().c_str(),
			(*toptr - *fromptr).toString().c_str());
		double	weight = (ignore_weights) ? 1. : *weightptr;
		// add coefficients to A array
		A[i        ] = fromptr->x() * weight;
		A[i +     m] = -fromptr->y() * weight;
		A[i + 2 * m] = 1 * weight;
		A[i + 3 * m] = 0;

                b[i] = toptr->x() * weight;

		i++;

		A[i        ] = fromptr->y() * weight;
		A[i +     m] = fromptr->x() * weight;
		A[i + 2 * m] = 0;
		A[i + 3 * m] = 1 * weight;

                b[i] = toptr->y() * weight;

		i++;

		fromptr++;
		toptr++;
		if (!ignore_weights) { weightptr++; }
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "number of equations: %d", i);

	// solve the linear system
	char	trans = 'N';
	int	n = 4;
	int	nrhs = 1;
	int	lda = m;
	int	ldb = m;
	int	lwork = -1;
	int	info = 0;

	// first call to dgels is set up to determine the needed size of the
	// work array.
	double	x;
	dgels_(&trans, &m, &n, &nrhs, A, &lda, b, &ldb, &x, &lwork, &info);
	if (info != 0) {
		std::string	msg = stringprintf("dgels cannot determine "
			"work area size: %d", info);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	lwork = x;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "need work area of size %d", lwork);

	// with the correct work array in place, the next call solves the
	// equations
	double	work[lwork];
	dgels_(&trans, &m, &n, &nrhs, A, &lda, b, &ldb, work, &lwork, &info);
	if (info != 0) {
		std::string	msg = stringprintf("dgels cannot solve "
			"equations: %d", info);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// now set up the transform
	Transform	t;

	// copy result data
	t[0] = b[0];
	t[1] = -b[1];
	t[2] = b[2];
	t[3] = b[1];
	t[4] = b[0];
	t[5] = b[3];

	debug(LOG_DEBUG, DEBUG_LOG, 0, "transformation found: %s",
		t.toString().c_str());

	// compute the residual
	double	residual = 0.;
	fromptr = from.begin();
	toptr = to.begin();
	i = 0;
	while (fromptr != from.end()) {
		double	X, Y;
		X = t[0] * fromptr->x() + t[1] * fromptr->y() + t[2];
		Y = t[3] * fromptr->x() + t[4] * fromptr->y() + t[5];
		double	delta = hypot(X - toptr->x(), Y - toptr->y());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "residual[%d] = %f",
			i++, delta);
		residual += delta;
		fromptr++;
		toptr++;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "residual = %f", residual);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "average residual %f",
		residual / from.size());

	return t;
} 

} // namespace transform
} // namespace image
} // namespace astro
