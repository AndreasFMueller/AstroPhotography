/*
 * EventMonitor.cpp -- event monitor implementation
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "EventMonitor.h"

namespace snowgui {

EventMonitor::EventMonitor() {
	qRegisterMetaType<snowstar::Event>("snowstar::Event");
}

void	EventMonitor::update(const snowstar::Event& event,
			const Ice::Current& /* current */) {
	emit updateSignal(event);
}

void	EventMonitor::stop(const Ice::Current& /* current */) {
	emit stopSignal();
}

} // namespace snowgui
