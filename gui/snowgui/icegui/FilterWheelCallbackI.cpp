/*
 * FilterWheelCallbackI.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <filterwheelcontrollerwidget.h>
#include <IceConversions.h>
#include <IceUtil/UUID.h>

namespace snowgui {

FilterWheelCallbackI::FilterWheelCallbackI(filterwheelcontrollerwidget& f)
	: _filterwheelcontrollerwidget(f) {
	_identity.name = IceUtil::generateUUID();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "identity constructed: %s",
		_identity.name.c_str());
}

FilterWheelCallbackI::~FilterWheelCallbackI() {
}

void	FilterWheelCallbackI::state(const snowstar::FilterwheelState state,
		const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new state");
	_filterwheelcontrollerwidget.callbackState(state);
}

void	FilterWheelCallbackI::position(const int position,
		const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new position %d", position);
	_filterwheelcontrollerwidget.callbackPosition(position);
}

void	FilterWheelCallbackI::stop(const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stop");
}

} // namespace snowgui
