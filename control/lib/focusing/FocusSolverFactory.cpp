/*
 * FocusSolverFactory.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroFocus.h>
#include <AstroUtils.h>
#include "FocusSolvers.h"

namespace astro {
namespace focusing {

/**
 * \brief Factory method to construct solvers
 *
 * \param solvername	name of the solver to retrieve
 */
FocusSolverPtr	FocusSolverFactory::get(const std::string& solvername) {
	FocusSolver	*solver = NULL;
	if (solvername == "centroid") {
		solver = new CentroidSolver();
	}
	if (solvername == "parabolic") {
		solver = new ParabolicSolver();
	}
	if (solvername == "abs") {
		solver = new AbsoluteValueSolver();
	}
	if (solvername == "maximum") {
		solver = new MaximumSolver();
	}
	if (solvername == "minimum") {
		solver = new MinimumSolver();
	}
	if (solvername == "brenner") {
		solver = new BrennerSolver();
	}
	if (NULL == solver) {
		std::string	msg = stringprintf("no solver for name '%s'",
			solvername.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	return FocusSolverPtr(solver);
}

/**
 * \brief Get a list of known solver names
 */
std::list<std::string>	FocusSolverFactory::solvernames() {
	std::list<std::string>	names;
	names.push_back(std::string("centroid"));
	names.push_back(std::string("parabolic"));
	names.push_back(std::string("abs"));
	names.push_back(std::string("maximum"));
	names.push_back(std::string("minimum"));
	names.push_back(std::string("brenner"));
	return names;
}

} // namespace focusing
} // namespace astro
