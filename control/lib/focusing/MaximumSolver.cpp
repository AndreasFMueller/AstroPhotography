/*
 * MaximumSolver.cpp -- solves by looking for the maximum
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroFocus.h>
#include "FocusSolvers.h"

namespace astro {
namespace focusing {

MaximumSolver::MaximumSolver() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create a new MaximumSolver");
}

int	MaximumSolver::position(const FocusItems& focusitems) {
	FocusItems::const_iterator	i;
	maximumposition = -1;
	maximum = 0;
	minimum = std::numeric_limits<float>::max();
	for (i = focusitems.begin(); i != focusitems.end(); i++) {
		float	value = i->value();
		if (value > maximum) {
			maximumposition = i->position();
			maximum = i->value();
		}
		if (value < minimum) {
			minimum = value;
		}
	}
	if (maximumposition < 0) {
		std::string	msg("maximum not found, not solvable");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::string(msg);
	}
	return maximumposition;
}

} // namespace focusing
} // namespace astro
