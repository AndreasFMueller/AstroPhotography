/*
 * CcdCallbackI.cpp -- Ccd state callback implementation
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <ccdcontrollerwidget.h>

namespace snowgui {

CcdCallbackI::CcdCallbackI(ccdcontrollerwidget& c) : _ccdcontrollerwidget(c) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ccd callback constructed");
}

void	CcdCallbackI::state(snowstar::ExposureState s, const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "received ccd state update: %d", s);
	emit stateChanged(s);
}

void	CcdCallbackI::stop(const Ice::Current& /* current */) {
}

} // namespace snowgui
