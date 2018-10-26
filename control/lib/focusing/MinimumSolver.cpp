/*
 * MinimumSolver.cpp -- solves by looking for the minimum
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroFocus.h>
#include "FocusSolvers.h"

namespace astro {
namespace focusing {

MinimumSolver::MinimumSolver() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create a new MinimumSolver");
}

int	MinimumSolver::position(const FocusItems& focusitems) {
	FocusItems::const_iterator	i;
	minimumposition = -1;
	minimum = 0;
	maximum = std::numeric_limits<float>::max();
	for (i = focusitems.begin(); i != focusitems.end(); i++) {
		float	value = i->value();
		if (value < minimum) {
			minimumposition = i->position();
			minimum = i->value();
		}
		if (value > maximum) {
			maximum = value;
		}
	}
	if (minimumposition < 0) {
		std::string	msg("minimum not found, not solvable");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::string(msg);
	}
	return minimumposition;
}

} // namespace focusing
} // namespace astro
