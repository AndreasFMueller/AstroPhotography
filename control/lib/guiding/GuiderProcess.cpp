/*
 * GuilderProcess.cpp -- GuiderProcess implementation 
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <GuiderProcess.h>
#include <AstroDebug.h>
#include <includes.h>
#include <AstroUtils.h>

using namespace astro::camera;
using namespace astro::image::transform;
using namespace astro::image;
using namespace astro::thread;

namespace astro {
namespace guiding {

/**
 * \brief create a GuiderProcess instance
 *
 * This also initializes the values for guider port activation to values that
 * compensate the drift to first order.
 */
GuiderProcess::GuiderProcess(Guider& _guider, double interval,
	persistence::Database _database)
	: guider(_guider), _interval(interval), database(_database)  {
	// set a default gain
	_gain = 1.;

}

/**
 * \brief Destroy the GuiderProcess
 *
 * This stops both threads, waits until they terminate, and cleans
 * up the resources allocated by them.
 */
GuiderProcess::~GuiderProcess() {
	// stop the thread specific data
	if (tracking) {
		tracking->stop();
		tracking->wait(_interval);
		tracking = NULL;
	}
	if (driving) {
		driving->stop();
		driving->wait(_interval);
		driving = NULL;
	}
	delete trackingwork;
	delete drivingwork;
}

/**
 * \brief Start the tracker process
 *
 * \param _tracker	the tracker to use to determine the offset
 */
bool	GuiderProcess::start(TrackerPtr _tracker) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "launching guiding threads");
	// create the driving process
	drivingwork = new DrivingWork(guider);
	driving = ThreadPtr(new astro::thread::Thread<DrivingWork>(*drivingwork));

	// create the tracking process
	trackingwork = new TrackingWork(guider, _tracker, *drivingwork);
	tracking = ThreadPtr(new astro::thread::Thread<TrackingWork>(*trackingwork));

	// start both processes
	driving->start();
	tracking->start();

	// that's it, we are done
	debug(LOG_DEBUG, DEBUG_LOG, 0, "guiding threads launched");
	return true;
}

/**
 * \brief Stop the tracker process
 */
bool	GuiderProcess::stop() {
	if (driving) {
		driving->stop();
	}
	if (tracking) {
		tracking->stop();
	}
	return true;
}

double	GuiderProcess::getGain() const {
	return _gain;
}

void	GuiderProcess::setGain(double gain) {
	_gain = gain;
}

bool	GuiderProcess::wait(double timeout) {
	bool	result = true;
	if (tracking) {
		result &= tracking->wait(timeout);
	}
	if (driving) {
		result &= tracking->wait(timeout);
	}
	return result;
}

void	GuiderProcess::lastAction(double& actiontime, Point& offset,
		Point& activation) {
	if (trackingwork) {
		trackingwork->lastAction(actiontime, offset, activation);
	}
}


} // namespace guiding
} // namespace astro
