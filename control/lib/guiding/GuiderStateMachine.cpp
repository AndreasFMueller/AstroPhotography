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
	return (_state == Guide::unconfigured);
}

bool	GuiderStateMachine::canStartGuiding() const {
	return (_state == Guide::calibrated);
}

bool	GuiderStateMachine::canAcceptCalibration() const {
	return (_state != Guide::guiding);
}

bool	GuiderStateMachine::canFailCalibration() const {
	return (_state != Guide::guiding);
}

bool	GuiderStateMachine::canStopGuiding() const {
	return (_state == Guide::guiding);
}

bool	GuiderStateMachine::canStartCalibrating() const {
	return (_state == Guide::calibrated)
		|| (_state == Guide::idle);
}

void	GuiderStateMachine::configure() {
	if (!canConfigure()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot configured in state %s",
			statename());
		throw BadState("cannot start calibration");
	}
	_state = Guide::idle;
}

void	GuiderStateMachine::startCalibrating() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start calibrating");
	if (!canStartCalibrating()) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"cannot start calibrating in state %s", statename());
		throw BadState("cannot start calibration");
	}
	_state = Guide::calibrating;
}

void	GuiderStateMachine::addCalibration() {
	if (!canAcceptCalibration()) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"cannot accept calibration in state %s", statename());
		throw BadState("cannot accept calibration in this state");
	}
	_state = Guide::calibrated;
}

void	GuiderStateMachine::failCalibration() {
	if (!canFailCalibration()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot fail calibration in state %s",
			statename());
		throw BadState("cannot fail calibration in this state");
	}
	_state = Guide::idle;
}

void	GuiderStateMachine::startGuiding() {
	if (!canStartGuiding()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot start guiding in state %s",
			statename());
		throw BadState("cannot start guiding in this state");
	}
	_state = Guide::guiding;
}

void	GuiderStateMachine::stopGuiding() {
	if (!canStopGuiding()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot stop guiding in state %s",
			statename());
		throw BadState("cannot stop guiding in this state");
	}
	_state = Guide::calibrated;
}

} // namespace guiding
} // namespace astro
