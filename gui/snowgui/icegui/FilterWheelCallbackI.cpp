/*
 * FilterWheelCallbackI.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <filterwheelcontrollerwidget.h>
#include <IceConversions.h>

namespace snowgui {

FilterWheelCallbackI::FilterWheelCallbackI(filterwheelcontrollerwidget& f)
	: _filterwheelcontrollerwidget(f) {
}

FilterWheelCallbackI::~FilterWheelCallbackI() {
}

void	FilterWheelCallbackI::state(const snowstar::FilterwheelState state,
		const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new state");
	_filterwheelcontrollerwidget.statusUpdate();
}

void	FilterWheelCallbackI::position(const int position,
		const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new position %d", position);
	_filterwheelcontrollerwidget.positionUpdate();
}

void	FilterWheelCallbackI::stop(const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stop");
}

} // namespace snowgui
