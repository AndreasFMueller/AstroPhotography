/*
 * GuiderStateMachine.cpp -- implementation of the guider state machine
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <GuiderStateMachine.h>

namespace Astro {

bool	GuiderStateMachine::canConfigure() const {
	return (_state == Astro::Guider::GUIDER_UNCONFIGURED);
}

bool	GuiderStateMachine::canStartGuiding() const {
	return (_state == Astro::Guider::GUIDER_IDLE);
}

bool	GuiderStateMachine::canAcceptCalibration() const {
	return (_state == Astro::Guider::GUIDER_UNCONFIGURED) ||
		(_state == Astro::Guider::GUIDER_IDLE) ||
		(_state == Astro::Guider::GUIDER_CALIBRATED);
}

bool	GuiderStateMachine::canStopGuiding() const {
	return (_state == Astro::Guider::GUIDER_GUIDING);
}

bool	GuiderStateMachine::canStartCalibrating() const {
	return (_state == Astro::Guider::GUIDER_CALIBRATED)
		|| (_state == Astro::Guider::GUIDER_IDLE);
}

void	GuiderStateMachine::configure() {
	if (!canConfigure()) {
		throw BadState("cannot start calibration");
	}
	_state = Astro::Guider::GUIDER_IDLE;
}

void	GuiderStateMachine::startCalibrating() {
	if (!canStartCalibrating()) {
		throw BadState("cannot start calibration");
	}
	_state = Astro::Guider::GUIDER_CALIBRATING;
}

void	GuiderStateMachine::addCalibration() {
	if (!canAcceptCalibration()) {
		throw BadState("cannot accept calibration in this state");
	}
	_state = Astro::Guider::GUIDER_CALIBRATED;
}

void	GuiderStateMachine::startGuiding() {
	if (!canStartGuiding()) {
		throw BadState("cannot start guiding in this state");
	}
	_state = Astro::Guider::GUIDER_GUIDING;
}

void	GuiderStateMachine::stopGuiding() {
	if (!canStopGuiding()) {
		throw BadState("cannot stop guiding in this state");
	}
	_state = Astro::Guider::GUIDER_CALIBRATED;
}

} // namespace Astro
