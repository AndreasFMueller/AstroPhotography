/*
 * TrackingProcess.cpp -- the tracking process
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroGuiding.h>
#include <TrackingProcess.h>
#include <TrackingPersistence.h>

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
	ControlDevicePtr guiderPortDevice,
	ControlDevicePtr adaptiveOpticsDevice,
	persistence::Database database)
	: BasicProcess(guider, tracker, database),
	  _guiderPortDevice(guiderPortDevice),
	  _adaptiveOpticsDevice(adaptiveOpticsDevice),
	  _summary(guider->name(), guider->instrument(), guider->ccdname()) {
	_gain = 1;
	_guiderportInterval = 10;
	_adaptiveopticsInterval = 0;
	_id = -1;

	// additional fields in the summary
	if (guiderPortDevice) {
		_summary.descriptor.guiderport(guiderPortDevice->devicename());
	}
	if (adaptiveOpticsDevice) {
		_summary.descriptor.guiderport(adaptiveOpticsDevice->devicename());
	}
	
	// install the callback
	TrackingProcessCallback	*trackingprocesscallback
		= new TrackingProcessCallback(this);
	_callback = CallbackPtr(trackingprocesscallback);
	guider->addTrackingCallback(_callback);

	// make sure there is a thread
        thread(ThreadPtr(new astro::thread::Thread<TrackingProcess>(this)));
}

TrackingProcess::~TrackingProcess() {
	if (_callback) {
		guider()->removeTrackingCallback(_callback);
	}
}

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

void	TrackingProcess::main(thread::Thread<TrackingProcess>& thread) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "TRACK: tracker main function started");

	// create a new record in the database
	if (database()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "TRACK: have database");
		GuidingRun      guidingrun;
		guidingrun.name = guider()->name();
		guidingrun.instrument = guider()->instrument();
		guidingrun.ccd = guider()->ccdname();
                guidingrun.guiderport = _guiderPortDevice->devicename();
                guidingrun.guiderportcalid
			= _guiderPortDevice->calibrationid();
		if (_adaptiveOpticsDevice) {
			guidingrun.adaptiveoptics
				= _adaptiveOpticsDevice->devicename();
			guidingrun.adaptiveopticscalid
				= _adaptiveOpticsDevice->calibrationid();
		} else {
			guidingrun.adaptiveopticscalid = -1;
		}
		time(&guidingrun.whenstarted);

                // add guiding run record to the database
		GuidingRunRecord        record(0, guidingrun);
		GuidingRunTable guidingruntable(database());
		_id = guidingruntable.add(record);
		_summary.trackingid = _id;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "TRACK %d: start", _id);
	}

	// get the interval for images
	double	imageInterval = _guiderportInterval;
	if (_adaptiveOpticsDevice) {
		if (_adaptiveOpticsDevice->iscalibrated()) {
			imageInterval = _adaptiveopticsInterval;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "TRACK %d: image interval: %.3fs", _id,
		imageInterval);

	// every time we go through the loop we ask whether we should terminate
	// we also do this at appropriate points within the loop
	double	guiderportTime = 0;
	while (!thread.terminate()) {
		// we measure the time it takes to get an exposure. This
		// may be larger than the interval, so we need the time
		// to protect from overcorrecting
		Timer	timer;
		timer.start();

		debug(LOG_DEBUG, DEBUG_LOG, 0, "TRACK %d: start new exposure",
			_id);

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
			goto cleanup;
		}

		// use the tracker to find the tracking offset
		Point	offset = tracker()->operator()(image);
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"TRACK %d: current tracker offset: %s", _id,
			offset.toString().c_str());
		_summary.addPoint(offset);

		// find out whether the tracker can still track, terminate
		// if not
		if ((offset.x() != offset.x()) || (offset.y() != offset.y())) {
			debug(LOG_ERR, DEBUG_LOG, 0,
				"TRACK %d: loss of tracking, give up", _id);
			goto cleanup;
		}

		// we modify the correction, which allows us to make
		// the correction more stable
		offset = offset * gain();
		Point	remainder  = offset;
		if (_adaptiveOpticsDevice) {
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
				"TRACK %d: correct by GP: %s",
				_id, remainder.toString().c_str());
		}

		// check whether enough time has passed for a guider port
		// action. Because there may be some variance in image 
		// acquisition, we subtract half the elapsed time of the last
		// image acquisition from the interval to ensure that there
		// really will be a guider port update within each guide
		// interval
		if (Timer::gettime() > guiderportTime + _guiderportInterval
			- timer.elapsed() / 2) {
			Point	d = _guiderPortDevice->correct(remainder,
				_guiderportInterval, _stepping);
			guiderportTime = Timer::gettime();
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"TRACK %d: guiderport remaining offset %s",
				_id, d.toString().c_str());
		}

		// time we want to sleep until the next AO action is waranted
		double	dt = imageTime + imageInterval - Timer::gettime();
		if (dt > 0) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "TRACK %d: sleep %.2f",
				_id, dt);
			Timer::sleep(dt);
		}
	}
cleanup:
	debug(LOG_DEBUG, DEBUG_LOG, 0, "TRACK %d: Termination signal received",
		_id);
	_id = -1;
}
	

} // namespace guiding
} // namespace astro
