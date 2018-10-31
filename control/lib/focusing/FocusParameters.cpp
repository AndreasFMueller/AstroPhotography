/*
 * FocusParameters.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroFocus.h>

namespace astro {
namespace focusing {

/**
 * \brief Construct the parameters with a reasonable interval
 */
FocusParameters::FocusParameters(unsigned long minposition,
	unsigned long maxposition)
	: _minposition(minposition), _maxposition(maxposition),
	  _steps(10), _method("fwhm"), _solver("abs") {
	if (_minposition >= _maxposition) {
		std::string	msg = stringprintf("empty interval %lu >= %lu",
			_minposition, _maxposition);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
}

/**
 * \brief Copy constructor
 */
FocusParameters::FocusParameters(const FocusParameters& parameters) 
	: _minposition(parameters._minposition),
	  _maxposition(parameters._maxposition),
	  _steps(parameters._steps),
	  _exposure(parameters._exposure),
	  _method(parameters._method),
	  _solver(parameters._solver) {
}

/**
 * \brief Set the number of steps
 *
 * This method ensures that the number of steps is reasonable
 */
void	FocusParameters::steps(int s) {
	if (s <= 1) {
		std::string	msg = stringprintf("focusing needs at least 2 "
			"steps, only %d specified", s);
		throw std::runtime_error(msg);
	}
	_steps = s;
}

/**
 * \brief Set the exposure
 *
 * This method ensures that the exposure structure has the purpose and
 * focus set correctly
 */
void	FocusParameters::exposure(const camera::Exposure& e) {
	_exposure = e;
	_exposure.purpose(camera::Exposure::focus);
	_exposure.shutter(camera::Shutter::OPEN);
}

/**
 * \brief Set the evaluator method
 *
 * This method ensures that only known evaluator methods can ever be set
 */
void	FocusParameters::method(const std::string& m) {
	std::list<std::string>	methods = FocusEvaluatorFactory::evaluatornames();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "known evaluators: %s",
		unsplit(methods, ", ").c_str());
	if (methods.end() == std::find(methods.begin(), methods.end(), m)) {
		std::string	msg = stringprintf("method '%s' not known",
			m.c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	_method = m;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found method: %s", _method.c_str());
}

/**
 * \brief Set the solver
 *
 * This method ensures that only known solvers can ever be set
 */
void	FocusParameters::solver(const std::string& s) {
	std::list<std::string>	solvers = FocusSolverFactory::solvernames();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "known solvers: %s",
		unsplit(solvers, ", ").c_str());
	if (solvers.end() == std::find(solvers.begin(), solvers.end(), s)) {
		std::string	msg = stringprintf("solver '%s' not known",
			s.c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	_solver = s;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found solver: %s", _solver.c_str());
}

} // namespace focusing
} // namespace astro
