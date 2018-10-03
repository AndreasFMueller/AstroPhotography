/*
 * StateMonitoringThread.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <ccdcontrollerwidget.h>
#include <image.h>
#include <AstroDebug.h>

namespace snowgui {

/**
 * \brief Create a State monitoring thread
 */
StateMonitoringThread::StateMonitoringThread(ccdcontrollerwidget *c)
	: QThread(NULL), _ccdcontrollerwidget(c) {
	_running = true;
}

/**
 * \brief stop a state monitoring thread
 */
StateMonitoringThread::~StateMonitoringThread() {
	_running = false;
	// can we speed up this process by sending this thread a signal?
	wait();
}

/**
 * \brief Main method doing the state monitoring
 */
void	StateMonitoringThread::run() {
	snowstar::ExposureState	previousstate = snowstar::IDLE;
	while (_running) {
		if (_ccdcontrollerwidget->_ccd) {
			snowstar::ExposureState	newstate
				= _ccdcontrollerwidget->_ccd->exposureStatus();
			if (newstate != previousstate) {
				emit stateChanged(newstate);
			}
			previousstate = newstate;
		}
		usleep(100000);
	}
}

} // namespace snowgui
