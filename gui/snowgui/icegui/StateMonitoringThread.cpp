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
	connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
}

/**
 * \brief stop a state monitoring thread
 */
StateMonitoringThread::~StateMonitoringThread() {
	_running = false;
}

/**
 * \brief Main method doing the state monitoring
 */
void	StateMonitoringThread::run() {
	snowstar::ExposureState	previousstate = snowstar::IDLE;
	while (_running) {
		usleep(100);
		std::lock_guard<std::recursive_mutex>	lock(_mutex);
		if ((_ccdcontrollerwidget) && (_ccdcontrollerwidget->_ccd)) {
			snowstar::ExposureState	newstate
				= _ccdcontrollerwidget->_ccd->exposureStatus();
			if (newstate != previousstate) {
				emit stateChanged(newstate);
			}
			previousstate = newstate;
		}
	}
}

/**
 * \brief Stop the thread
 */
void	StateMonitoringThread::stop() {
	std::lock_guard<std::recursive_mutex>	lock(_mutex);
	_running = false;
	_ccdcontrollerwidget = NULL;
}

} // namespace snowgui
