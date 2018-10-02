/*
 * StateMonitoringThread.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <ccdcontrollerwidget.h>
#include <image.h>
#include <AstroDebug.h>

namespace snowgui {

StateMonitoringThread::StateMonitoringThread(ccdcontrollerwidget *c)
	: QThread(NULL), _ccdcontrollerwidget(c) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "thread created");
	_running = true;
}

StateMonitoringThread::~StateMonitoringThread() {
	_running = false;
	wait();
}

void	StateMonitoringThread::run() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "thread started");
	snowstar::ExposureState	previousstate = snowstar::IDLE;
	while (_running) {
		if (_ccdcontrollerwidget->_ccd) {
			snowstar::ExposureState	newstate
				= _ccdcontrollerwidget->_ccd->exposureStatus();
			debug(LOG_DEBUG, DEBUG_LOG, 0, "state: %d", newstate);
			if (newstate != previousstate) {
				debug(LOG_DEBUG, DEBUG_LOG, 0, "state change");
				emit stateChanged(newstate);
			}
			previousstate = newstate;
		}
		usleep(100000);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "thread terminated");
}

} // namespace snowgui
