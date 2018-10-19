/*
 * ParabolicFocusSolver.cpp -- find the maximum assuming parabola
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroFocus.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <lapack.h>

namespace astro {
namespace focusing {

ParabolicSolver::ParabolicSolver() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating parabolic solver");
}

/**
 * \brief Compute the minumum position of a set of focus items
 *
 * This method assumes that the values form a parabola, so we try to
 * fit a parabola and then use the symmetry axis of the parabola as
 * the solution to the focus problem.
 */
int	ParabolicSolver::position(const FocusItems& focusitems) {
	// allocate memory for the equation
	int	m = focusitems.size();
	if (m < 3) {
		std::string	msg = stringprintf("not enough data (%d < 3) "
					"to compute a focus position", m);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	double	A[3 * m];
	double	b[m];

	// fill the array and right hand side
	FocusItems::const_iterator	i;
	int	j;
	for (i = focusitems.begin(), j = 0; i != focusitems.end(); i++, j++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "adding %d, %f",
			i->position(), i->value());
		double	x = i->position();
		A[j        ] = 1;
		A[j +     m] = x;
		A[j + 2 * m] = x * x;
		b[j] = i->value();
	}

	// solve the system of equations
	char	trans = 'N';
	int	n = 3;
	int	nrhs = 1, lda = m, ldb = m, info = 0;
	double	worksize;
	double	*work = &worksize;
	int	lwork = -1;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "trans = %c, m = %d, n = %d, nrhs = %d, "
		"A = %p, lda = %d, b = %p, ldb = %d, work = %p, lwork = %d, "
		"info = %d",
		trans, m, n, nrhs, A, lda, b, ldb, work, lwork, info);
	dgels_(&trans, &m, &n, &nrhs, A, &lda, b, &ldb, work, &lwork, &info);
	if (0 != info) {
		debug(LOG_ERR, DEBUG_LOG, 0, "i = %d", info);
		throw std::runtime_error("cannot determine solution");
	}
	lwork = worksize;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "allocating workspace of size %d",
		lwork);
	work = (double *)alloca(lwork * sizeof(double));
	dgels_(&trans, &m, &n, &nrhs, A, &lda, b, &ldb, work, &lwork, &info);
	if (0 != info) {
		throw std::runtime_error("cannot determine solution");
	}

	// optimal solution 
	debug(LOG_DEBUG, DEBUG_LOG, 0, "a0 = %.6f, a1 = %.6f, a2 = %.6f",
		b[0], b[1], b[2]);
	int	pos = -b[1] / (2 * b[2]);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found position %d", pos);
	return pos;
}

} // namespace focusing
} // namespace astro
