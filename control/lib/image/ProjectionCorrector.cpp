/*
 * ProjectionCorrector.cpp -- compute a correction to a 
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroProjection.h>

#ifdef HAVE_ACCELERATE_ACCELERATE_H
#include <Accelerate/Accelerate.h>
#else
#include <lapack.h>
#endif /* HAVE_ACCELERATE_ACCELERATE_H */

namespace astro {
namespace image {
namespace transform {

/**
 * \brief get the corrected projection
 */
Projection	ProjectionCorrector::corrected(
			const std::vector<Residual>& residuals) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "correct projection: %s",
		centeredprojection.toString().c_str());
	// allocate an array for the computation of the correction
	int	n = 2 * residuals.size();
	double	a[8 * n];
	double	b[n];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d equations", n);

	// start filling in the arrays
	std::vector<Residual>::const_iterator	r;
	unsigned int	i;
	for (r = residuals.begin(), i = 0; r != residuals.end(); r++, i += 2) {
		b[i    ] = r->offset().x();
		b[i + 1] = r->offset().y();
	}

	// display the right hand side
	if (debuglevel >= LOG_DEBUG) {
		std::string	msg("b = [\n");
		for (i = 0; i < 2 * residuals.size(); i++) {
			msg += stringprintf("%f;\n", b[i]);
		}
		msg += std::string("];\n");
		std::cout << msg;
	}

	// build the matrix of derivatives
	double	h = 0.01;
	for (int j = 0; j < 8; j++) {
		CenteredProjection	pr = centeredprojection;
		pr[j] += h;
		for (r = residuals.begin(), i = 0; r != residuals.end();
			r++, i += 2) {
			Point	p1 = centeredprojection(r->from());
			Point	p2 = pr(r->from());
			Point	delta = (p2 - p1) * (1 / h);
			a[i     + n * j] = delta.x();
			a[i + 1 + n * j] = delta.y();
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "derivative matrix computed");

	// report the derivatives matrix for debugging
	if (debuglevel >= LOG_DEBUG) {
		std::string	msg("A = [\n");
		for (i = 0; i < 2 * residuals.size(); i++) {
			msg += stringprintf("/* %3d */ ", i);
			for (int j = 0; j < 8; j++) {
				if (j > 0) { msg += std::string(","); }
				msg += stringprintf(" %10.3g", a[i + n * j]);
			}
			msg += stringprintf(";\n");
		}
		msg += std::string("];\n");
		std::cout << msg;
	}

	// apply the weights
	for (r = residuals.begin(), i = 0; r != residuals.end(); r++, i++) {
		b[2 * i    ] *= r->weight();
		b[2 * i + 1] *= r->weight();
		for (unsigned int j = 0; j < 8; j++) {
			a[2 * i     + n * j] *= r->weight();
			a[2 * i + 1 + n * j] *= r->weight();
		}
	}

	// compute liwork (based on dgelsd documentation)
	int	nlvl = 4; /* LOG_2(8 / SMLSIZ + 1) + 1 */
	int	liwork = (3 * nlvl + 11) * 8;

	// solve the system of equations, gives the correction
	int	m = 8;
	int	nrhs = 1;
	int	lda = n;
	int	ldb = n;
	double	s[8];
	double	rcond = 0;
	int	rank = 0;
	int	lwork = -1;
	int	iwork_length = liwork;
	int	info = 0;
	double	x;
	dgelsd_(&n, &m, &nrhs, a, &lda, b, &ldb, s, &rcond, &rank, &x, &lwork,
		&iwork_length, &info);
	if (info != 0) {
		std::string     msg = stringprintf("dgels cannot determine "
			"work area size: %d", info);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	lwork = x;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "need work area of size %d, "
		"and iwork of size %d", lwork, iwork_length);

	// with the correct work array in place, the next call solves the
	// equations
	double  work[lwork];
	int	iwork[iwork_length];
	dgelsd_(&n, &m, &nrhs, a, &lda, b, &ldb, s, &rcond, &rank, work, &lwork,
		iwork, &info);
	if (info != 0) {
		std::string     msg = stringprintf("dgels cannot solve "
			"equations: %d", info);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	if (info == 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "reported iwork length: %d",
			iwork[0]);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "system of equations solved, rank = %d",
		rank);
	if (debuglevel >= LOG_DEBUG) {
		for (int j = 0; j < 8; j++) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"singular value[%d] = %f", j, s[j]);
		}
	}

        // copy result vector
	CenteredProjection	pr = centeredprojection;
	for (int j = 0; j < 8; j++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "correction[%d] = %g", j, b[j]);
		pr[j] += b[j];
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "original projection: %s",
		centeredprojection.toString().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new projection: %s",
		pr.toString().c_str());

	return pr;
}

} // namespace project
} // namespace image
} // namespace astro
