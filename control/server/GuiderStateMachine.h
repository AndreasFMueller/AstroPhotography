/*
 * GuiderStateMachine.h -- state machine for guider implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _GuiderStateMachine_h
#define _GuiderStateMachine_h

#include <guider.hh>

namespace Astro {

class GuiderStateMachine {
	Guider::GuiderState	_state;
public:
	const Guider::GuiderState&	state() const { return _state; }
	operator Guider::GuiderState () { return _state; }

	// construct the state machine
	GuiderStateMachine() : _state(Astro::Guider::GUIDER_UNCONFIGURED) { }

	// methods to find out whether we can accept a configuration, or
	// start calibration or guiding
	bool	canConfigure() const;
	bool	canStartGuiding() const;
	bool	canStartCalibrating() const;
	bool	canAcceptCalibration() const;
	bool	canFailCalibration() const;
	bool	canStopGuiding() const;

	// state change methods
	void	configure();
	void	startCalibrating();
	void	addCalibration();
	void	startGuiding();
	void	stopGuiding();
};

} // namespace Astro

#endif /* _GuiderStateMachine_h */
