/*
 * FilterWheelCallbackI.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <filterwheelcontrollerwidget.h>
#include <IceConversions.h>
#include <IceUtil/UUID.h>

namespace snowgui {

FilterWheelCallbackI::FilterWheelCallbackI() {
}

void	FilterWheelCallbackI::state(const snowstar::FilterwheelState state,
		const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new state");
	//_filterwheelcontrollerwidget.callbackState(state);
	emit callbackState(state);
}

void	FilterWheelCallbackI::position(const int position,
		const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new position %d", position);
	//_filterwheelcontrollerwidget.callbackPosition(position);
	emit callbackPosition(position);
}

void	FilterWheelCallbackI::stop(const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stop");
}

} // namespace snowgui
