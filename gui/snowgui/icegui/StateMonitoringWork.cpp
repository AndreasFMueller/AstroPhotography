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
	_previousstate = snowstar::IDLE;
	connect(this,
		SIGNAL(stateChanged(snowstar::ExposureState)),
		_ccdcontrollerwidget,
		SLOT(statusUpdate(snowstar::ExposureState)));
}

/**
 * \brief stop a state monitoring thread
 */
StateMonitoringWork::~StateMonitoringWork() {
}

/**
 * \brief Main method doing the state monitoring
 */
void	StateMonitoringWork::updateStatus() {
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "updateStatus()");
	if (!_ccdcontrollerwidget->_ccd) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no ccd");
		return;
	}
	try {
		snowstar::ExposureState	newstate
			= _ccdcontrollerwidget->_ccd->exposureStatus();
		if (newstate != _previousstate) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"state change detected, new state %d",
				newstate);
			emit stateChanged(newstate);
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"stateChanged(%d) emitted", newstate);
		}
		_previousstate = newstate;
	} catch (const std::exception& x) {
		std::string	msg = astro::stringprintf(
			"cannot get ccd state: %s", x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	}
}

} // namespace snowgui
