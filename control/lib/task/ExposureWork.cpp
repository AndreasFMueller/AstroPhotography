/*
 * ExposureWork.cpp -- Work to be done for a single exposure
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroTask.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>
#include <AstroLoader.h>
#include <AstroDevaccess.h>
#include <ImageDirectory.h>
#include <errno.h>
#include <string.h>
#include <AstroIO.h>
#include <ExposureWork.h>

using namespace astro::persistence;
using namespace astro::io;

namespace astro {
namespace task {

/**
 * \brief Create a work object
 *
 * The constructor sets up the devices used for task execution. This should
 * not take any noticable time, in particular this can be done synchronously.
 */
ExposureWork::ExposureWork(TaskQueueEntry& task) : _task(task) {
	// create a repository, we are always using the default
	// repository
	astro::module::Repository	repository;
	
	// get camera and ccd
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get camera '%s' and ccd %d",
		_task.camera().c_str(), _task.ccdid());
	astro::device::DeviceAccessor<astro::camera::CameraPtr>
		dc(repository);
	camera = dc.get(_task.camera());
	ccd = camera->getCcd(_task.ccdid());

	// turn on the cooler
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set cooler to %f",
		_task.ccdtemperature());
	if ((ccd->hasCooler()) && (_task.ccdtemperature() > 0)) {
		cooler = ccd->getCooler();
	}

	// get the filterwheel
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get filter '%s' of wheel '%s'",
		_task.filter().c_str(), _task.filterwheel().c_str());
	astro::device::DeviceAccessor<astro::camera::FilterWheelPtr>
		df(repository);
	if ((_task.filterwheel().size() > 0) && (_task.filter().size() > 0)) {
		filterwheel = df.get(_task.filterwheel());
	}

	// if the task does not have a frame size, then take the one from the
	// the CCD
	if (_task.size() == ImageSize()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "using the full chip");
		_task.frame(ccd->getInfo().getFrame());
	}

	// that's it, the task is constructed
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ExposureWork created");
}

/**
 * \brief Condition to wait for the cooler to stabilize
 */
class CoolerCondition : public Condition {
	camera::CoolerPtr	_cooler;
public:
	CoolerCondition(camera::CoolerPtr cooler) : _cooler(cooler) { }
	virtual bool	operator()() { return _cooler->stable(); }
};

/**
 * \brief Condition to wait for a filterwheel to reach a certain state
 */
class FilterwheelCondition : public Condition {
	camera::FilterWheelPtr	_filterwheel;
	camera::FilterWheel::State	_state;
public:
	FilterwheelCondition(camera::FilterWheelPtr filterwheel,
		camera::FilterWheel::State state)
		: _filterwheel(filterwheel), _state(state) { }
	virtual bool	operator()() {
		return (_filterwheel->getState() == _state);
	}
};

/**
 * \brief Condition to wait for the CCD to complete the exposure
 */
class CcdCondition : public Condition {
	camera::CcdPtr	_ccd;
	camera::Exposure::State	_state;
public:
	CcdCondition(camera::CcdPtr ccd, camera::Exposure::State state)
		: _ccd(ccd), _state(state) { }
	virtual bool	operator()() {
		return (_ccd->exposureStatus() == _state);
	}
};

/**
 * \brief Task execution function
 *
 * In this function, all waits should use the wait function to ensure that
 * cancel is recognized.
 */
void	ExposureWork::run() {
	// set the cooler
	if (cooler) {
		cooler->setTemperature(_task.ccdtemperature());
		cooler->setOn(true);
	}

	// set the filterwheel position
	std::string	filtername("NONE");
	if (filterwheel) {
		// make sure filter wheel is ready
		FilterwheelCondition	condition(filterwheel,
			camera::FilterWheel::idle);
		if (!wait(10., condition)) {
			throw std::runtime_error("filterwheel did not settle");
		}

		// select the new filter
		filterwheel->select(_task.filter());
		filtername = _task.filter();
	}

	// wait for the cooler, if present, but at most 30 seconds
	if (cooler) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "wait for cooler");
		CoolerCondition	coolercondition(cooler);
		if (wait(30., coolercondition)) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"cannot stabilize temperature");
			// XXX what do we do when the cooler cannot stabilize?
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cooler now stable");
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no cooler");
	}

	// wait for the filterwheel if present
	if (filterwheel) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "wait for filterwheel");

		// wait once more for the filterwheel to become ready
		FilterwheelCondition	condition(filterwheel,
			camera::FilterWheel::idle);
		if (!wait(30., condition)) {
			throw std::runtime_error("filter wheel does not idle");
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "filterwheel now idle");
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no filter");
	}

	// start exposure
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start exposure: time=%f",
		_task.exposure().exposuretime());
	ccd->startExposure(_task.exposure());

	// wait for completion of exposure
	debug(LOG_DEBUG, DEBUG_LOG, 0, "waiting for %.3f seconds",
		_task.exposure().exposuretime());
	CcdCondition	ccdcondition(ccd, camera::Exposure::exposed);

	// if waiting is cancelled, then we have to cancel the exposure
	// also
	try {
		if (!wait(_task.exposure().exposuretime() + 30, ccdcondition)) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"waiting for image failed");
			throw std::runtime_error("failed waiting for image");
		}
	} catch (CancelException& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cancel exception caught");
		// cancel the image...
		ccd->cancelExposure();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "exposure cancelled, waiting");

		// ... and wait until either the camera is in a safe state
		do {
			sleep(1);
			
		} while ((ccd->exposureStatus() == camera::Exposure::cancelling)
			|| (ccd->exposureStatus() == camera::Exposure::exposing));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "wait comlete");

		// now throw again to signal the work process that the process
		// was in fact cancelled
		throw;
	}


	// get the image from the ccd
	astro::image::ImagePtr	image = ccd->getImage();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image frame: %s",
		image->getFrame().toString().c_str());

	// add filter information to the image, if present
	image->setMetadata(FITSKeywords::meta("FILTER", filtername));

	// add temperature metadata
	if (cooler) {
		cooler->addTemperatureMetadata(*image);
	}

	// add to the ImageDirectory
	astro::image::ImageDatabaseDirectory	imagedir;
	std::string	filename = imagedir.save(image);

	// remember the filename
	_task.filename(filename);

	// update the frame information
	astro::camera::Exposure	exposure = _task.exposure();
	_task.exposure(exposure);

	// copy the returned image information
	_task.size(image->size());
	_task.origin(image->origin());

	// log info
	debug(LOG_DEBUG, DEBUG_LOG, 0, "file %s written",
		_task.filename().c_str());
	_task.state(TaskQueueEntry::complete);
}

/**
 * \brief Destroy the exposure work
 *
 * Ensure that the devices are reset back to a reasonable state, in particular,
 * we should turn off the cooler.
 */
ExposureWork::~ExposureWork() {
	// turn of the cooler
	if (cooler) {
		cooler->setOn(false);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ExposureWork destroyed");
}

} // namespace task
} // namespace astro
