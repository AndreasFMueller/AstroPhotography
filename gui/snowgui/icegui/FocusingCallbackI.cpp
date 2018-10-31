/*
 * FocusingCallbackI.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <FocusingCallbackI.h>
#include <AstroDebug.h>

namespace snowgui {

FocusingCallbackI::FocusingCallbackI() : QObject(NULL) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "callback created");

	qRegisterMetaType<snowstar::FocusElement>("snowstar::FocusElement");
	qRegisterMetaType<snowstar::FocusPoint>("snowstar::FocusPoint");
	qRegisterMetaType<snowstar::FocusState>("snowstar::FocusState");
}

FocusingCallbackI::~FocusingCallbackI() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "callback destroyed");
}

void	FocusingCallbackI::addPoint(const snowstar::FocusPoint& point,
		const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adding point");
	emit pointReceived(point);
}

void	FocusingCallbackI::changeState(const snowstar::FocusState state,
		const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "changing state");
	emit stateReceived(state);
}

void	FocusingCallbackI::addFocusElement(const snowstar::FocusElement& element,
		const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adding focus element");
	emit focuselementReceived(element);
}

} // namespace snowgui
