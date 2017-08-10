/*
 * LinearRegression.cpp
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "LinearRegression.h"
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <stdexcept>

#ifdef HAVE_ACCELERATE_ACCELERATE_H
#include <Accelerate/Accelerate.h>
#else
#include <lapack.h>
#endif /* HAVE_ACCELERATE_ACCELERATE_H */

namespace astro {
namespace linear {

/**
 * \brief
 */
LinearRegression::LinearRegression(const std::vector<std::pair<double, double> >& data) {
	int	n = data.size();
	int	m = 2;
	double	A[n * m];
	double	b[n];
	std::vector<std::pair<double, double> >::const_iterator	i;
	i = data.begin();
	int	j = 0;
	while (i != data.end()) {
		A[j    ] = i->first;
		A[j + n] = 1;
		b[j] = i->second;
		j++;
		i++;
	}

	// prepare to solve the system using LAPACK (dgels_)
	char	trans = 'N';
	int	nrhs = 1;
	int	lda = n;
	int	ldb = n;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "lda = %d, ldb = %d", lda, ldb);
	int	lwork = -1;
	int	info = 0;

        // determine work area size
	double  x;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "solving a %d x %d system", n, m);
	dgels_(&trans, &n ,&m, &nrhs, A, &lda, b, &ldb, &x, &lwork, &info);
	if (info != 0) {
		std::string     msg = stringprintf("dgels cannot determine "
			"work area size: %d", info);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	lwork = x;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "need work area of size %d", lwork);

	//  allocate work array
	double  work[lwork];
	dgels_(&trans, &n ,&m, &nrhs, A, &lda, b, &ldb, work, &lwork, &info);
	if (info != 0) {
		std::string     msg = stringprintf("dgels cannot solve "
			"equations: %d", info);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"solution: %5.1f %5.1f", b[0], b[1]);
	_a = b[0];
	_b = b[1];
}

} // namespace linear
} // namespace astro
