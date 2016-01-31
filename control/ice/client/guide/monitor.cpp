/*
 * monitor.cpp -- command line client to control guiding
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hohschule Rapperswil
 */
#include "monitor.h"
#include "display.h"
#include <guider.h>
#include <iostream>
#include <AstroDebug.h>
#include <CommunicatorSingleton.h>
#include <CommonClientTasks.h>
#include "guide.h"

namespace snowstar {
namespace app {
namespace snowguide {

//////////////////////////////////////////////////////////////////////
// Calibration Monitor implementation
//////////////////////////////////////////////////////////////////////

CalibrationMonitorI::CalibrationMonitorI() : display(std::cout) {
}

void	CalibrationMonitorI::update(const CalibrationPoint& point,
	const Ice::Current& /* current */) {
	display(point);
}

void	CalibrationMonitorI::stop(const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stop received");
	complete(true);
}

//////////////////////////////////////////////////////////////////////
// Tracking Monitor implementation
//////////////////////////////////////////////////////////////////////

TrackingMonitorI::TrackingMonitorI() : display(std::cout, 0) {
}

bool	TrackingMonitorI::csv() const {
	return display.csv();
}

void	TrackingMonitorI::csv(bool c) {
	display.csv(c);
}

void	TrackingMonitorI::update(const TrackingPoint& point,
	const Ice::Current& /* current */) {
	display(point);
}

void	TrackingMonitorI::stop(const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stop received");
	complete(true);
}

//////////////////////////////////////////////////////////////////////
// monitoring commands in the Guide class
//////////////////////////////////////////////////////////////////////
static Guide	*guide = NULL;

void    signal_handler(int /* sig */) {
	if (guide) {
		guide->complete(true);
	}
}

void	Guide::complete(bool c) {
	if (monitor) {
		monitor->complete(c);
	}
}

int	Guide::monitor_calibration(GuiderPrx guider) {
	debug(LOG_INFO, DEBUG_LOG, 0, "monitoring calibration");
	// create a new calibration monitor
	CalibrationMonitorI	*calmonitor = new CalibrationMonitorI();

	// register the monitor with the guider server
	Ice::ObjectPtr	callback = calmonitor;
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
	CallbackAdapter	adapter(ic);
	Ice::Identity	ident = adapter.add(callback);
	guider->ice_getConnection()->setAdapter(adapter.adapter());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "register calibration monitor");
	guider->registerCalibrationMonitor(ident);

	// monitor
	monitor = calmonitor;
	guide = this;

	// wait for termination of the monitor
	debug(LOG_DEBUG, DEBUG_LOG, 0, "waiting for calibration completion");
	calmonitor->wait();

	guide = NULL;
	monitor = NULL;

	// unregister the monitor
	guider->unregisterCalibrationMonitor(ident);
	return EXIT_SUCCESS;
}

int	Guide::monitor_guiding(GuiderPrx guider) {
	debug(LOG_INFO, DEBUG_LOG, 0, "monitoring guiding");
	// create a new calibration monitor
	TrackingMonitorI	*trackmonitor = new TrackingMonitorI();
	trackmonitor->csv(csv);

	// register the monitor with the guider server
	Ice::ObjectPtr	callback = trackmonitor;
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
	CallbackAdapter	adapter(ic);
	Ice::Identity	ident = adapter.add(callback);
	guider->ice_getConnection()->setAdapter(adapter.adapter());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "register tracking monitor");
	guider->registerTrackingMonitor(ident);

	monitor = trackmonitor;
	guide = this;

	// wait for termination of the monitor
	debug(LOG_DEBUG, DEBUG_LOG, 0, "waiting for guiding completion");
	trackmonitor->wait();

	guide = NULL;
	monitor = NULL;

	// unregister the monitor
	guider->unregisterTrackingMonitor(ident);
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation of the images command
 */
int	Guide::images_command(GuiderPrx guider, const std::string& path) {
	// create a Image callback object
	GuideImageMonitor	*guidemonitor
		= new GuideImageMonitor(path, prefix);
	Ice::ObjectPtr	callback = guidemonitor;

	// register the callback with the adapter
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
	CallbackAdapter	adapter(ic);
	Ice::Identity	ident = adapter.add(callback);
	guider->ice_getConnection()->setAdapter(adapter.adapter());

	// register the image callback with the server
	debug(LOG_DEBUG, DEBUG_LOG, 0, "registering image callback");
	guider->registerImageMonitor(ident);

	monitor = guidemonitor;
	guide = this;
	
	// wait until the callback gets the information that the process
	// completed
	debug(LOG_DEBUG, DEBUG_LOG, 0, "wait for monitor to complete");
	guidemonitor->wait();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "monitor completed");

	guide = NULL;
	monitor = NULL;

	// unregister the callback before exiting
	guider->unregisterImageMonitor(ident);
	return EXIT_SUCCESS;
}

} // namespace snowguide
} // namespace app
} // namespace snowstar

