/*
 * TrackingMonitorController.cpp -- implementation of tracking monitor
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <TrackingMonitorController.h>
#include <AstroDebug.h>
#include <CommunicatorSingleton.h>
#include <CommonClientTasks.h>

namespace snowgui {

/**
 * \brief Construct a tracking monitor controller
 */
TrackingMonitorController::TrackingMonitorController(QObject *parent,
	trackingmonitordialog *dialog) : QObject(parent), _dialog(dialog) {

	// connect the signal
	connect(this, SIGNAL(dataUpdated()), this, SLOT(refreshDisplay()),
                Qt::QueuedConnection);
}

/**
 * \brief Destroy the tracking monitor controller
 */
TrackingMonitorController::~TrackingMonitorController() {
	if (_guider) {
		try {
			_guider->unregisterTrackingMonitor(_myidentity);
		} catch (const std::exception& x) {
			debug(LOG_ERR, DEBUG_LOG, 0,
				"cannot unregister tracking monitor %s: %s",
				_myidentity.name.c_str(), x.what());
		}
	}
}

/**
 * \brief Register with the server
 */
void    TrackingMonitorController::setGuider(snowstar::GuiderPrx guider,
                                Ice::ObjectPtr myself) {
	_guider = guider;
	snowstar::CommunicatorSingleton::connect(_guider);
	_myidentity = snowstar::CommunicatorSingleton::add(myself);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "identity: %s",
		_myidentity.name.c_str());
	guider->registerTrackingMonitor(_myidentity);
}

/**
 * \brief Callback method for stop
 */
void	TrackingMonitorController::stop(const Ice::Current&) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stop received");
	// XXX currently we do nothing, we should somehow indicate that
	// XXX no more data will bi forthcoming
}

/**
 * \brief Callback method for tracking point updates
 *
 * This method does all the processing that is allowed in a separate thread
 * and then emits the signal to perform the display update
 */
void	TrackingMonitorController::update(const snowstar::TrackingPoint& point,
		const Ice::Current&) {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"new tracking point received (timeago = %.1f)", point.timeago);
	_dialog->add(point);
	emit dataUpdated();
}

/**
 * \brief Slot to refresh the display with the new data
 */
void	TrackingMonitorController::refreshDisplay() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "refresh slot called");
	_dialog->refreshDisplay();
}


} // namespace snowgui
