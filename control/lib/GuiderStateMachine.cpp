/*
 * GuiderStateMachine.cpp -- implementation of the guider state machine
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroDebug.h>
#include <AstroGuiding.h>

namespace astro {
namespace guiding {

static const char	*statenames[5] = {
	"unconfigured", "idle", "calibrating", "calibrated", "guding"
};

const char	*GuiderStateMachine::statename() const {
	return statenames[_state];
}

bool	GuiderStateMachine::canConfigure() const {
	return (_state == astro::guiding::unconfigured);
}

bool	GuiderStateMachine::canStartGuiding() const {
	return (_state == astro::guiding::calibrated);
}

bool	GuiderStateMachine::canAcceptCalibration() const {
	return (_state != astro::guiding::guiding);
}

bool	GuiderStateMachine::canStopGuiding() const {
	return (_state == astro::guiding::guiding);
}

bool	GuiderStateMachine::canStartCalibrating() const {
	return (_state == astro::guiding::calibrated)
		|| (_state == astro::guiding::idle);
}

void	GuiderStateMachine::configure() {
	if (!canConfigure()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot configured in state %s",
			statename());
		throw std::runtime_error("cannot start calibration");
	}
	_state = astro::guiding::idle;
}

void	GuiderStateMachine::startCalibrating() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start calibrating");
	if (!canStartCalibrating()) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"cannot start calibrating in state %s", statename());
		throw std::runtime_error("cannot start calibration");
	}
	_state = astro::guiding::calibrating;
}

void	GuiderStateMachine::addCalibration() {
	if (!canAcceptCalibration()) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"cannot accept calibration in state %s", statename());
		throw std::runtime_error("cannot accept calibration in this state");
	}
	_state = astro::guiding::calibrated;
}

void	GuiderStateMachine::startGuiding() {
	if (!canStartGuiding()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot start guiding in state %s",
			statename());
		throw std::runtime_error("cannot start guiding in this state");
	}
	_state = astro::guiding::guiding;
}

void	GuiderStateMachine::stopGuiding() {
	if (!canStopGuiding()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot stop guiding in state %s",
			statename());
		throw std::runtime_error("cannot stop guiding in this state");
	}
	_state = astro::guiding::calibrated;
}

} // namespace guiding
} // namespace astro
