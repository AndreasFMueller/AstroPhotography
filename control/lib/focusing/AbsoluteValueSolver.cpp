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

int	AbsoluteValueSolver::position(const FocusItems& focusitems) const {
	FocusItems	squareditems;
	FocusItems::const_iterator	i;
	for (i = focusitems.begin(); i != focusitems.end(); i++) {
		squareditems.insert(FocusItem(i->position(), sqr(i->value())));
	}
	return ParabolicSolver::position(squareditems);
}

AbsoluteValueSolver::AbsoluteValueSolver() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating AbsoluteValueSolver");
}

} // namespace focusing
} // namespace astro

