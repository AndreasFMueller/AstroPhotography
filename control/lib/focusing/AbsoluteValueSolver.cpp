/*
 * AbsoluteValueSolver.cpp
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroFocus.h>

namespace astro {
namespace focusing {

static inline float	sqr(float x) {
	return x * x;
}

/**
 * \brief Find the solution of the focus problem 
 *
 * This solver assumes that the values are an absolute value function 
 * of the position. This means that the squares form a parabola, so we
 * use the parabolic solver to find the focus position.
 */
int	AbsoluteValueSolver::position(const FocusItems& focusitems) {
	FocusItems	squareditems;
	FocusItems::const_iterator	i;
	for (i = focusitems.begin(); i != focusitems.end(); i++) {
		squareditems.insert(FocusItem(i->position(), sqr(i->value())));
	}
	return ParabolicSolver::position(squareditems);
}

/**
 * \brief Create the absolute value solver
 */
AbsoluteValueSolver::AbsoluteValueSolver() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating AbsoluteValueSolver");
}

} // namespace focusing
} // namespace astro

