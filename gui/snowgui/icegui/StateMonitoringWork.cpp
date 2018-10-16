/*
 * StateMonitoringWork.cpp
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
StateMonitoringWork::StateMonitoringWork(ccdcontrollerwidget *c)
	: QObject(NULL), _ccdcontrollerwidget(c) {
	_running = true;
}

/**
 * \brief stop a state monitoring thread
 */
StateMonitoringWork::~StateMonitoringWork() {
	_running = false;
}

/**
 * \brief Main method doing the state monitoring
 */
void	StateMonitoringWork::updateStatus() {
	snowstar::ExposureState	previousstate = snowstar::IDLE;
	std::lock_guard<std::recursive_mutex>	lock(_mutex);
	if ((_ccdcontrollerwidget) && (_ccdcontrollerwidget->_ccd)) {
		try {
			snowstar::ExposureState	newstate
				= _ccdcontrollerwidget->_ccd->exposureStatus();
			if (newstate != previousstate) {
				emit stateChanged(newstate);
			}
			previousstate = newstate;
		} catch (const std::exception& x) {
			std::string	msg = astro::stringprintf(
				"cannot get ccd state: %s", x.what());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		}
	}
}

/**
 * \brief Stop the thread
 */
void	StateMonitoringWork::stop() {
	std::lock_guard<std::recursive_mutex>	lock(_mutex);
	_running = false;
	_ccdcontrollerwidget = NULL;
}

} // namespace snowgui
