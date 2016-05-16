/*
 * CentroidSolver.cpp
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroFocus.h>

namespace astro {
namespace focusing {

int	CentroidSolver::position(const FocusItems& /* focusitems */) const {
	
};

CentroidSolver::CentroidSolver() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating Centroid solver");
}

} // namespace focusing
} // namespace astro
