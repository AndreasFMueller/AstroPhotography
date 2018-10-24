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
	if (methods.end() == std::find(methods.begin(), methods.end(), m)) {
		std::string	msg = stringprintf("method '%s' not known",
			m.c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	_method = m;
}

/**
 * \brief Set the solver
 *
 * This method ensures that only known solvers can ever be set
 */
void	FocusParameters::solver(const std::string& s) {
	std::list<std::string>	solvers = FocusSolverFactory::solvernames();
	if (solvers.end() == std::find(solvers.begin(), solvers.end(), s)) {
		std::string	msg = stringprintf("solver '%s' not known",
			s.c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	_solver = s;
}

/**
 * \brief String to state conversion
 */
FocusParameters::state_type FocusParameters::string2state(const std::string& s) {
	if (s == "idle") {
		return FocusProcess::IDLE;
	}
	if (s == "moving") {
		return FocusProcess::MOVING;
	}
	if (s == "measuring") {
		return FocusProcess::MEASURING;
	}
	if (s == "focused") {
		return FocusProcess::FOCUSED;
	}
	if (s == "failed") {
		return FocusProcess::FAILED;
	}
	throw std::runtime_error("bad focus status");
}

/**
 * \brief State to string conversion
 */
std::string	FocusParameters::state2string(state_type s) {
	switch (s) {
	case IDLE:
		return std::string("idle");
		break;
	case MOVING:
		return std::string("moving");
		break;
	case MEASURING:
		return std::string("measuring");
		break;
	case FOCUSED:
		return std::string("focused");
		break;
	case FAILED:
		return std::string("failed");
		break;
	}
	throw std::runtime_error("bad focus status");
}

} // namespace focusing
} // namespace astro
