/*
 * GuiderStateMachine.cpp -- implementation of the guider state machine
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroDebug.h>
#include <AstroGuiding.h>

namespace astro {
namespace guiding {

static const char	*statenames[8] = {
	"unconfigured", "idle", "calibrating", "calibrated", "guiding",
                "darkacquire", "flatacquire", "imaging"
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

bool	GuiderStateMachine::canStartDarkAcquire() const {
	return (_state == Guide::idle) || (_state == Guide::calibrated);
}

bool	GuiderStateMachine::canEndDarkAcquire() const {
	return (_state == Guide::darkacquire);
}

bool	GuiderStateMachine::canStartFlatAcquire() const {
	return (_state == Guide::idle) || (_state == Guide::calibrated);
}

bool	GuiderStateMachine::canEndFlatAcquire() const {
	return (_state == Guide::flatacquire);
}

bool	GuiderStateMachine::canStartImaging() const {
	return (_state == Guide::idle) || (_state == Guide::calibrated);
}

bool	GuiderStateMachine::canEndImaging() const {
	return (_state == Guide::imaging);
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

void	GuiderStateMachine::startDarkAcquire() {
	if (!canStartDarkAcquire()) {
		std::string	msg = stringprintf("cannot dark acquire in "
			"state %s", statename());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw BadState(msg);
	}
	_prestate = _state;
	_state = Guide::darkacquire;
}

void	GuiderStateMachine::endDarkAcquire() {
	if (!canEndDarkAcquire()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "Not acquiring dark image");
		throw BadState("Not acquireing dark image");
	}
	_state = _prestate;
}

void	GuiderStateMachine::startFlatAcquire() {
	if (!canStartFlatAcquire()) {
		std::string	msg = stringprintf("cannot flat acquire in "
			"state %s", statename());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw BadState(msg);
	}
	_prestate = _state;
	_state = Guide::flatacquire;
}

void	GuiderStateMachine::endFlatAcquire() {
	if (!canEndFlatAcquire()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "Not acquiring flat image");
		throw BadState("Not acquireing flat image");
	}
	_state = _prestate;
}

void	GuiderStateMachine::startImaging() {
	if (!canStartImaging()) {
		std::string	msg = stringprintf("cannot image in state %s",
			statename());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw BadState(msg);
	}
	_prestate = _state;
	_state = Guide::imaging;
}

void	GuiderStateMachine::endImaging() {
	if (!canEndImaging()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "Not imaging");
		throw BadState("Not imaging");
	}
	_state = _prestate;
}

} // namespace guiding
} // namespace astro
