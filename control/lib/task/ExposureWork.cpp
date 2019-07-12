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
#include <AstroProject.h>
#include <AstroConfig.h>
#include <AstroFormat.h>
#include <ImageDirectory.h>
#include <errno.h>
#include <string.h>
#include <AstroIO.h>
#include <AstroGateway.h>
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
ExposureWork::ExposureWork(TaskQueueEntry& __task) : TaskWork(__task) {
	if (__task.taskType() != tasktype(tasktype::EXPOSURE)) {
		std::string	msg = stringprintf("%d is not an exposure task",
			__task.id());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::logic_error(msg);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "constructing Work object for task %s",
		task().toString().c_str());
	// create a repository, we are always using the default
	// repository
	astro::module::ModuleRepositoryPtr	repository
		= module::getModuleRepository();
	
	// get camera and ccd
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get camera '%s' and ccd %s",
		task().camera().c_str(), task().ccd().c_str());
	try {
		astro::device::DeviceAccessor<astro::camera::CameraPtr>
			dc(repository);
		camera = dc.get(task().camera());
	} catch (const std::exception& x) {
		std::string	msg = stringprintf("cannot get camera: %s",
			x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw;
	}
	try {
		astro::device::DeviceAccessor<astro::camera::CcdPtr>
			dc(repository);
		ccd = camera->getCcd(task().ccd());
	} catch (const std::exception& x) {
		std::string	msg = stringprintf("cannot get ccd: %s",
			x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw;
	}

	// turn on the cooler
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get cooler '%s', temperature %.2f ",
		task().cooler().c_str(), task().ccdtemperature());
	if ((task().cooler().size() > 0) && (task().ccdtemperature() > 0)) {
		try {
			astro::device::DeviceAccessor<astro::camera::CoolerPtr>
				df(repository);
			cooler = df.get(task().cooler());
		} catch (std::exception& x) {
			std::string	msg = stringprintf("cannot get cooler: %s",
				x.what());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw;
		}
	}

	// get the filterwheel
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get filter '%s' of wheel '%s'",
		task().filter().c_str(), task().filterwheel().c_str());
	if (task().filterwheel().size() > 0) {
		try {
			astro::device::DeviceAccessor<astro::camera::FilterWheelPtr>
				df(repository);
			filterwheel = df.get(task().filterwheel());
		} catch (const std::exception& x) {
			std::string	msg = stringprintf("cannot get filterwheel: %s",
				x.what());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw;
		}
	}

	// get the mount
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get mount %s", task().mount().c_str());
	if (task().mount().size() > 0) {
		try {
			astro::device::DeviceAccessor<astro::device::MountPtr>
				df(repository);
			mount = df.get(task().mount());
		} catch (const std::exception& x) {
			std::string	msg = stringprintf("cannot get mount: %s",
				x.what());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw;
		}
	}

	// get the focuser
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get focuser %s", task().focuser().c_str());
	if (task().focuser().size() > 0) {
		try {
			astro::device::DeviceAccessor<astro::camera::FocuserPtr>
				df(repository);
			focuser = df.get(task().focuser());
		} catch (const std::exception& x) {
			std::string	msg = stringprintf("cannot get focuser: %s",
				x.what());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw;
		}
	}

	// if the task does not have a frame size, then take the one from the
	// the CCD
	if (task().size() == ImageSize()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "using the full chip");
		task().frame(ccd->getInfo().getFrame());
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
	camera::CcdState::State	_state;
public:
	CcdCondition(camera::CcdPtr ccd, camera::CcdState::State state)
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start ExposureWork on task %d",
		task().id());
	// work with the instrument
	std::string	instrument = task().instrument();

	// set the cooler
	if (cooler) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "turning on cooler");
		cooler->setTemperature(task().ccdtemperature());
		cooler->setOn(true);
	}

	// set the filterwheel position
	std::string	filtername("NONE");
	if (filterwheel) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "selecting filter");
		// make sure filter wheel is ready
		FilterwheelCondition	condition(filterwheel,
			camera::FilterWheel::idle);
		if (!wait(10., condition)) {
			throw std::runtime_error("filterwheel did not settle");
		}

		// select the new filter
		if (task().filter().size() > 0) {
			filterwheel->select(task().filter());
			filtername = task().filter();
		}
	}

	// wait for the cooler, if present, but at most 30 seconds
	if (cooler) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "wait for cooler");
		CoolerCondition	coolercondition(cooler);
		if (!wait(30., coolercondition)) {
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
		task().exposure().exposuretime());
	ccd->startExposure(task().exposure());

	// record the current task information. This may take some time,
	// so we keep the time so that we can later correct the wait time
	// for exposure completion
	Timer	gatewaytime;
	gatewaytime.start();
	gateway::Gateway::update(instrument, (int)task().id());	// currenttaskid
	gateway::Gateway::updateImageStart(instrument);		// lastimagestart
	gateway::Gateway::update(instrument, task().project());	// project
	gateway::Gateway::update(instrument, task().exposure());	// exosuretime
	gateway::Gateway::update(instrument, filterwheel);	// filter
	gateway::Gateway::update(instrument, cooler);		// ccdtemperature
	gateway::Gateway::update(instrument, mount);		// position
	gateway::Gateway::update(instrument, focuser);		// focus

	gateway::Gateway::send(instrument);
	gatewaytime.end();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "gateway time took %.3f seconds",
		gatewaytime.elapsed());

	// cmopute the remaining time to wait
	double	waittime = task().exposure().exposuretime()
			- gatewaytime.elapsed();
	if (waittime < 0) {
		waittime = 0.001;
	}

	// wait for completion of exposure
	debug(LOG_DEBUG, DEBUG_LOG, 0, "waiting for %.3f seconds", waittime);
	CcdCondition	ccdcondition(ccd, camera::CcdState::exposed);

	// if waiting is cancelled, then we have to cancel the exposure
	// also
	try {
		if (!wait(waittime + 30, ccdcondition)) {
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
			
		} while ((ccd->exposureStatus() == camera::CcdState::cancelling)
			|| (ccd->exposureStatus() == camera::CcdState::exposing));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "wait comlete");

		// now throw again to signal the work process that the process
		// was in fact cancelled
		throw;
	}

	// get the image from the ccd
	astro::image::ImagePtr	image = ccd->getImage();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image frame: %s",
		image->getFrame().toString().c_str());

	// add instrument info
	if (task().instrument().size() > 0) {
		image->setMetadata(FITSKeywords::meta("INSTRUME",
			task().instrument()));
	}

	// add filter information to the image, if present
	if ((filterwheel) && (filtername.size() > 0)) {
		image->setMetadata(FITSKeywords::meta("FILTER", filtername));
	}

	// add temperature metadata
	if (cooler) {
		cooler->addTemperatureMetadata(*image);
	}

	// add focus metadata
	if (focuser) {
		focuser->addFocusMetadata(*image);
	}

	// position information from the mount
	if (mount) {
		mount->addPositionMetadata(*image);
	}

	// project inforamtion
	if (task().project().size() > 0) {
		image->setMetadata(astro::io::FITSKeywords::meta(
			std::string("PROJECT"), task().project()));
	}

	if (task().repository().size() > 0) {
		// add the image to the repository
		std::string	repodb = task().repodb();
		std::string	repository = task().repository();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "saving image to repo %s@%s",
			repository.c_str(), repodb.c_str());

		// build the image repo from the repo db name
		config::ConfigurationPtr	config;
		if (repodb.size() > 0) {
			config = config::Configuration::get(repodb);
		} else {
			config = config::Configuration::get();
		}
		project::ImageRepoPtr	imagerepo = config::ImageRepoConfiguration::get(config)->repo(repository);
		if (imagerepo) {
			long	id = imagerepo->save(image);
			task().filename(stringprintf("%ld", id));
		} else {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "no image repo found");
		}
	} else {
		// add to the ImageDirectory
		astro::image::ImageDatabaseDirectory	imagedir;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "saving image");
		std::string	filename = imagedir.save(image);

		// remember the filename
		task().filename(filename);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "saving image to file %s",
			filename.c_str());
	}

	// update the frame information
	astro::camera::Exposure	exposure = task().exposure();
	task().exposure(exposure);

	// copy the returned image information
	task().size(image->size());
	task().origin(image->origin());

	// log info
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image %s written",
		task().filename().c_str());
	task().state(TaskQueueEntry::complete);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "finish ExposureWork for task %d",
		task().id());
}

/**
 * \brief Destroy the exposure work
 *
 * Ensure that the devices are reset back to a reasonable state, in particular,
 * we should turn off the cooler.
 */
ExposureWork::~ExposureWork() {
	// turn of the cooler
	// XXX we should make this configurable, but for the time being
	// XXX we disable it, the cooler can still be turned off manually
#if 0
	if (cooler) {
		try {
			cooler->setOn(false);
		} catch (const std::exception& x) {
			debug(LOG_ERR, DEBUG_LOG, 0,
				"cannot turn off the cooler, giving up");
		}
	}
#endif
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ExposureWork destroyed");
}

} // namespace task
} // namespace astro
