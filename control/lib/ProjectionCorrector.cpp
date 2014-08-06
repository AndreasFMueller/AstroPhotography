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
Projection	ProjectionCorrector::corrected() const {
	// allocate an array for the computation of the correction
	int	n = 2 * residuals.size();
	double	a[8 * n];
	double	b[n];

	// start filling in the arrays
	std::vector<Residual>::const_iterator	r;
	int	i;
	for (r = residuals.begin(), i = 0; r != residuals.end(); r++, i += 2) {
		b[i    ] = r->offset().x();
		b[i + 1] = r->offset().y();
	}

	double	h = 0.01;
	for (int j = 0; j < 8; j++) {
		Projection	pr = projection;
		pr[j] += h;
		for (r = residuals.begin(), i = 0; r != residuals.end();
			r++, i += 16) {
			Point	p1 = projection(r->from());
			Point	p2 = pr(r->from());
			Point	delta = (p2 - p1) * (1 / h);
			a[j + i    ] = delta.x();
			a[j + i + 8] = delta.y();
		}
	}

	// solve the system of equations, gives the correction
	char	trans = 'N';
	int	m = 8;
	int	nrhs = 1;
	int	lda = n;
	int	ldb = n;
	int	lwork = -1;
	int	info = 0;
	double	x;
	dgels_(&trans, &n, &m, &nrhs, a, &lda, b, &ldb, &x, &lwork, &info);
	if (info != 0) {
		std::string     msg = stringprintf("dgels cannot determine "
			"work area size: %d", info);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	lwork = x;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "need work area of size %d", lwork);

	// with the correct work array in place, the next call solves the
	// equations
	double  work[lwork];
	dgels_(&trans, &m, &n, &nrhs, a, &lda, b, &ldb, work, &lwork, &info);
	if (info != 0) {
		std::string     msg = stringprintf("dgels cannot solve "
			"equations: %d", info);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

        // copy result vector
	Projection	pr = projection;
	for (int j = 0; j < 8; j++) {
		pr[j] += b[j];
	}

	return pr;
}

} // namespace project
} // namespace image
} // namespace astro
