/*
 * TrackingProcess.cpp -- the tracking process
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroGuiding.h>
#include "TrackingProcess.h"
#include "TrackingPersistence.h"

using namespace astro::callback;
using namespace astro::thread;

namespace astro {
namespace guiding {

/**
 * \brief Callback class for tracking points
 */
class TrackingProcessCallback : public Callback {
	TrackingProcess	*_process;
public:
	TrackingProcessCallback(TrackingProcess *process)
		: _process(process) {
	}
	CallbackDataPtr	operator()(CallbackDataPtr data);
};

/**
 * \brief callback operator to send tracking point to the process callback
 */
CallbackDataPtr	TrackingProcessCallback::operator()(CallbackDataPtr data) {
	TrackingPoint	*tpp = dynamic_cast<TrackingPoint *>(&*data);
	if (NULL != tpp) {
		_process->callback(*tpp);
	}
	return data;
}

/**
 * \brief construct a new Tracking Process
 */
TrackingProcess::TrackingProcess(GuiderBase *guider, TrackerPtr tracker,
	ControlDevicePtr guidePortDevice,
	ControlDevicePtr adaptiveOpticsDevice,
	persistence::Database database,
	FilterMethod _filter_method)
	: BasicProcess(guider, tracker, database),
	  _guidePortDevice(guidePortDevice),
	  _adaptiveOpticsDevice(adaptiveOpticsDevice),
	  _summary(guider->instrument()) {
	_filter_parameters[0] = 1;
	_filter_parameters[1] = 1;
	_guideportInterval = 10;
	_adaptiveopticsInterval = 0;
	_id = -1;
	_control = NULL;

	// construct the filter method thingy
	if (_guidePortDevice) {
		switch (_filter_method) {
		case FilterNONE:
			_control = new ControlBase(_guideportInterval);
			break;
		case FilterGAIN:
			_control = new GainControl(_guideportInterval);
			break;
		case FilterKALMAN:
			_control = new OptimalControl(_guideportInterval);
			break;
		}
		_control->filter_parameter(0, filter_parameter(0));
		_control->filter_parameter(1, filter_parameter(1));
	}

	// install the callback
	TrackingProcessCallback	*trackingprocesscallback
		= new TrackingProcessCallback(this);
	_callback = CallbackPtr(trackingprocesscallback);
	guider->addTrackingCallback(_callback);

	// make sure there is a thread
        thread(ThreadPtr(new astro::thread::Thread<TrackingProcess>(this)));
}

/**
 * \brief Destroy the tracking process
 */
TrackingProcess::~TrackingProcess() {
	if (_callback) {
		guider()->removeTrackingCallback(_callback);
	}
	if (_control) {
		delete _control;
		_control = NULL;
	}
}

/**
 * \brief Callback called when a new trackingpoint becomes available
 */
void	TrackingProcess::callback(const TrackingPoint& trackingpoint) {
	if (!database()) {
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "TRACK %d: store point %s", _id,
			trackingpoint.toString().c_str());
	// add point to table
	TrackingPointRecord	tracking(0, _id, trackingpoint);
	TrackingTable	trackingtable(database());
	trackingtable.add(tracking);
}

/**
 * \brief Find out whether the adaptive optics device is usable
 *
 * For this the device has to be present and configured
 */
bool	TrackingProcess::adaptiveOpticsUsable() {
	if (!_adaptiveOpticsDevice) {
		return false;
	}
	return _adaptiveOpticsDevice->iscalibrated();
}

/**
 * \brief Find out whether the guider port is usable
 *
 * Like for the adaptive optics device, for this the device has to be
 * present and configured
 */
bool	TrackingProcess::guidePortUsable() {
	if (!_guidePortDevice) {
		return false;
	}
	return _guidePortDevice->iscalibrated();
}

/**
 * \brief Exception class used to signal termination
 */
class TrackingTerminationException : public std::exception {
public:
	TrackingTerminationException() { }
	const char	*what() const throw() {
		return "tracking termination request";
	}
};

/**
 * \brief Main function of the tracking process
 */
void	TrackingProcess::main(thread::Thread<TrackingProcess>& thread) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "TRACK: tracker main function started");

	// create a new record in the database
	if (database()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "TRACK: have database");
		Track      track;
		track.instrument = guider()->instrument();
		track.ccd = guider()->ccdname();
		if (guidePortUsable()) {
                	track.guideport = _guidePortDevice->devicename();
			track.guideportcalid
				= _guidePortDevice->calibrationid();
		} else {
			track.guideportcalid = -1;
		}
		if (adaptiveOpticsUsable()) {
			track.adaptiveoptics
				= _adaptiveOpticsDevice->devicename();
			track.adaptiveopticscalid
				= _adaptiveOpticsDevice->calibrationid();
		} else {
			track.adaptiveopticscalid = -1;
		}
		time(&track.whenstarted);

                // add guiding run record to the database
		TrackRecord	record(0, track);
		TrackTable	tracktable(database());
		_id = tracktable.add(record);
		_summary.trackingid = _id;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "TRACK %d: start", _id);
	}

	// get the interval for images
	double	imageInterval = _guideportInterval;
	if (adaptiveOpticsUsable()) {
		if (_adaptiveOpticsDevice->iscalibrated()) {
			imageInterval = _adaptiveopticsInterval;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "TRACK %d: image interval: %.3fs", _id,
		imageInterval);

	// every time we go through the loop we ask whether we should terminate
	// we also do this at appropriate points within the loop
	double	guideportTime = 0;
	while (!thread.terminate()) {
		try {
			step(thread, imageInterval, guideportTime);
		} catch (const TrackingTerminationException& tte) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"TRACK %d terminated: %s", _id, tte.what());
			goto cleanup;
		} catch (const std::runtime_error& ex) {
			std::string	msg = stringprintf(
				"TRACK %d terminated by %s: %s", _id,
				demangle(typeid(ex).name()).c_str(), ex.what());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw ex;
		}
	}
cleanup:
	debug(LOG_DEBUG, DEBUG_LOG, 0, "TRACK %d: Termination signal received",
		_id);
	_id = -1;
}

/**
 * \brief Perform a single tracking step
 *
 * Tracking happens in individual steps. Each step takes an image,
 * computes the offset using the tracker, decides whether to use the
 * adaptive optics unit, sends the offset there, receives what remains
 * to be corrected, and sends that remainder to the guider port, but only
 * if it alreay is time to update the guider port (the guider port cannot
 * follow very fast update rates like the adaptive optics unit).
 */
void	TrackingProcess::step(thread::Thread<TrackingProcess>& thread,
		double imageInterval,
		double& guideportTime) {
	// we measure the time it takes to get an exposure. This
	// may be larger than the interval, so we need the time
	// to protect from overcorrecting
	Timer	timer;
	timer.start();

	debug(LOG_DEBUG, DEBUG_LOG, 0, "TRACK %d: start new exposure", _id);

	// now retrieve the image. This method has as a side
	// effect that the image is sent to the image callback
	double	imageTime = Timer::gettime();
	ImagePtr	image = guider()->getImage();
	timer.end();
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"TRACK %d: new image received, elapsed = %f", _id,
		timer.elapsed());

	// we may have received the terminate signal since we
	// started the image
	if (thread.terminate()) {
		throw TrackingTerminationException();
	}

	// use the tracker to find the tracking offset
	TrackerPtr	t = tracker();
	if (!t) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no tracker");
		return;
	}
	Point	offset = tracker()->operator()(image);
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"TRACK %d: current tracker offset: %s", _id,
		offset.toString().c_str());
	_summary.addPoint(offset);

	// ask the tracker for a processed image
	if (tracker()->processedImage()) {
		guider()->updateImage(tracker()->processedImage());
	}

	// find out whether the tracker can still track, terminate
	// if not
	if ((offset.x() != offset.x()) || (offset.y() != offset.y())) {
		std::string	cause = stringprintf("TRACK %d: loss of "
			"tracking, give up", _id);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
		throw std::runtime_error(cause);
	}

	// get the filtered offset
	if (_control) {
		offset = _control->correct(offset);
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"TRACK %d: filtered offset: %s", _id,
			offset.toString().c_str());
	}

	// now distribute the corrections to the different control devices
	Point	remainder  = offset;
	if (adaptiveOpticsUsable()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"TRACK %d: correct by AO: %s",
			_id, offset.toString().c_str());

		// do the correction using the adaptive optics device
		remainder = _adaptiveOpticsDevice->correct(offset,
			_adaptiveopticsInterval, _stepping);
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"TRACK %d: offset remaining after AO: %s", _id,
			remainder.toString().c_str());
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"TRACK %d: no AO, correct by GP: %s",
			_id, remainder.toString().c_str());
	}

	// if we have a usable guider port, give it the remaining correction
	if (guidePortUsable()) {
		// check whether enough time has passed for a guider port
		// action. Because there may be some variance in image 
		// acquisition, we subtract half the elapsed time of the last
		// image acquisition from the interval to ensure that there
		// really will be a guider port update within each guide
		// interval
		if (Timer::gettime() > guideportTime + _guideportInterval
			- timer.elapsed() / 2) {
			Point	d = _guidePortDevice->correct(remainder,
				_guideportInterval, _stepping);
			guideportTime = Timer::gettime();
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"TRACK %d: guideport leaves offset %s",
				_id, d.toString().c_str());
		}
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"TRACK %d: no usable guider port", _id);
	}

	// time we want to sleep until the next AO action is waranted
	double	dt = imageTime + imageInterval - Timer::gettime();
	if (dt > 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "TRACK %d: sleep %.2f",
			_id, dt);
		Timer::sleep(dt);
	}
}

float	TrackingProcess::filter_parameter(int index) const {
	return _filter_parameters[index];
}

void	TrackingProcess::filter_parameter(int index, float p) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new parameter[%d] = %f", index, p);
	_filter_parameters[index] = p;
	if (_control) {
		_control->filter_parameter(index, p);
	}
}

Point	TrackingProcess::filter_parameter() const {
	return Point(_filter_parameters[0], _filter_parameters[1]);
}


} // namespace guiding
} // namespace astro
