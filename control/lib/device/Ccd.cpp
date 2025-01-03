/*
 * Ccd.cpp -- Ccd implementation
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCamera.h>
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <AstroExceptions.h>
#include <AstroIO.h>
#include <AstroUtils.h>
#include <includes.h>
#include <sstream>
#include <mutex>
#include <condition_variable>

using namespace astro::image;
using namespace astro::io;

namespace astro {
namespace camera {

DeviceName::device_type	Ccd::devicetype = DeviceName::Ccd;

const static bool	ccd_lck_debug = false;

/**
 * \brief Construct a CCD device
 *
 * \param _info		The information about this CCD
 */
Ccd::Ccd(const CcdInfo& _info)
	: astro::device::Device(_info.name(), DeviceName::Ccd),
	  info(_info), _state(CcdState::idle) {

	// add parameter descriptors for focal length and limiting magnitude
	device::ParameterDescription    focallength_description(
						"focallength", 0.01, 4.0);
	Device::add(focallength_description);
	device::ParameterDescription    azimuth_description(
						"azimuth", 0.0, 360.0);
	Device::add(azimuth_description);
	device::ParameterDescription    limit_magnitude_description(
						"limit_magnitude", 0.0, 16.0);
	Device::add(limit_magnitude_description);

	// get focal length parameter
	debug(LOG_DEBUG, DEBUG_LOG, 0, "querying 'focallength' for %s",
		name().toString().c_str());
	float   focallength = 1.111;
	if (hasProperty("focallength")) {
		try {
			std::string	fl = getProperty("focallength");
			debug(LOG_DEBUG, DEBUG_LOG, 0, "got focallength: '%s'",
				fl.c_str());
			focallength = std::stod(fl);
		} catch (const std::exception& x) {
			debug(LOG_ERR, DEBUG_LOG, 0,
				"cannot get focal length: %s", x.what());
		}
	}
	try {
		parameter("focallength", focallength);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "using focallength %.3f[m]",
			parameterValueFloat("focallength"));
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "focal length unusable: %s",
			x.what());
	}

	// get the azimuth parameter 
	debug(LOG_DEBUG, DEBUG_LOG, 0, "querying 'azimuth' for %s",
		name().toString().c_str());
	float   azimuth = 1.111;
	if (hasProperty("azimuth")) {
		try {
			std::string	az = getProperty("azimuth");
			debug(LOG_DEBUG, DEBUG_LOG, 0, "got azimuth: '%s'",
				az.c_str());
			azimuth = std::stod(az);
		} catch (const std::exception& x) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "azimuth unusable: %s",
				x.what());
		}
	}
	try {
		parameter("azimuth", azimuth);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "using azimuth %.3f[°]",
			parameterValueFloat("azimuth"));
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "focal length unusable: %s",
			x.what());
	}

	// get limit magnitude parameter
	float   limit_magnitude = 11.111;
	if (hasProperty("limit_magnitude")) {
		try {
		limit_magnitude = std::stod(getProperty("limit_magnitude"));
		} catch (const std::exception& x) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"limit_magnitude unusable: %s", x.what());
		}
	}
	parameter("limit_magnitude", limit_magnitude);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "using limit magnitude %.2f",
		limit_magnitude);
}

/**
 * \brief Get the state
 */
CcdState::State	Ccd::state() {
	return _state;
}

/**
 * \brief Set the state, notify threads waiting for a state change
 *
 * This method changes the state. If the state is really different, then
 * all the threads waiting for a state change are notified. It is expected
 * that waiting threads will for themselves check whether the right state
 * has been reached and will wait again if not.
 *
 * \param s	the new state
 */
void	Ccd::state(CcdState::State s) {
	if (ccd_lck_debug)
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"LCK changing state to %s state(s)", 
			CcdState::state2string(s).c_str());

	// acquire a lock to protect the state
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	if (ccd_lck_debug)
		debug(LOG_DEBUG, DEBUG_LOG, 0, "--> LCK acquired state(s)");

	if (_state != s) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "state change %s -> %s",
			CcdState::state2string(_state).c_str(),
			CcdState::state2string(s).c_str());
		_state = s;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "notify all of state change");
		// notify waiting threads of the state change
		_condition.notify_all();
		// also update all the callbacks
		stateUpdate(s);
	}

	if (ccd_lck_debug)
		debug(LOG_DEBUG, DEBUG_LOG, 0, "<-- LCK released state(s)");
}

/**
 * \brief Start an exposure
 *
 * Initiate an exposure. The base class method performs some common
 * sanity checks (e.g. it will not accept subframes that don't fit within
 * the CCD area), and it will reject requests if an exposure is already in
 * progress. Derived classes should override this methode, but they should
 * call this method as the first step in their implementation, because
 * this method also sets up the infrastructure for the wait method.
 */
void    Ccd::startExposure(const Exposure& _exposure) {
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "LCK start exposure %s startExposure()",
	//	_exposure.toString().c_str());
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "--> LCK acquired startExposure()");
	// make sure we are in the right state, and only accept new exposures
	// in that state. This is important because if we change the
	// exposure member while an exposure is in progress, we may run into
	// trouble while doing the readout. 
	if (CcdState::idle != state()) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"start exposure only in idle state");
		throw BadState("start exposure only in idle state");
	}

	// copy the exposure info
	exposure = _exposure;

	// if the size was not specified in the exposure, take the full
	// CCD size
        if (exposure.size() == ImageSize(0, 0)) {
                exposure.frame(getInfo().getFrame());
        }
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start exposure: %s -> %s",
		_exposure.toString().c_str(),
		exposure.toString().c_str());

	// check that the frame to be exposed fits into the CCD
        if (!info.size().bounds(exposure.frame())) {
		debug(LOG_ERR, DEBUG_LOG, 0, "exposure does not fit in ccd");
		exposure.frame(info.size().containing(exposure.frame()));
        }

#if 1
	// make sure the exposure time is in the interval specified in 
	// the ccdinfo
	if (exposure.exposuretime() > info.maxexposuretime()) {
		exposure.exposuretime(info.maxexposuretime());
	}
	if (exposure.exposuretime() < info.minexposuretime()) {
		exposure.exposuretime(info.minexposuretime());
	}
#else
	if ((exposure.exposuretime() < info.minexposuretime())
		|| (exposure.exposuretime() > info.maxexposuretime())) {
		std::string	msg = stringprintf("exposure time %f out of "
			"range [%f,%f]",  exposure.exposuretime(),
			info.minexposuretime(),
			info.maxexposuretime());
		debug(LOG_ERR, DEBUG_LOG, 0, "bad exposure time range");
	}
#endif

	// remember the start time of the exposure, this will be useful
	// if we later want to wait for the exposure to complete.
	time(&lastexposurestart);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "exposure started at %d",
		lastexposurestart);
	state(CcdState::exposing);
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "<-- LCK released startExposure()");
}

/**
 * \brief Monitor progress of an exposure
 *
 * Find out whether an exposure is in progress. Optional method.
 */
CcdState::State Ccd::exposureStatus() {
	return state();
}

/**
 * \brief Cancel an exposure
 *
 * Note that some cameras cannot cancel an exposure other than by
 * resetting the camera, which will affect other CCDs of the same
 * camera as well. If you plan to implement this function for such
 * a camera,
 * make sure that you would usually read from the camera is also
 * stored locally so that it can be restored after the reset.
 */
void    Ccd::cancelExposure() {
	throw NotImplemented("cancelExposure not implemented");
}


/**
 * \brief Predicate class to detect CCD state changes
 */
class CcdStateChange {
	Ccd	*_ccd;
	CcdState::State	_s;
public:
	CcdStateChange(Ccd *ccd, CcdState::State s) : _ccd(ccd), _s(s) {
	}
	bool	operator()() {
		return _s != _ccd->exposureStatus();
	}
};

/**
 * \brief Waiting for completion is generic (except possibly for UVC cameras)
 *
 * This method returns true if exposure completes and an image is now 
 * available. 
 */
bool	Ccd::wait() {
	//debug(LOG_DEBUG, DEBUG_LOG, 0,
	//	"LCK waiting for image completion wait()");
	// lock the _mutex, so we are shure the state variable will not
	// change between checks
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "--> LCK acquired wait()");

	// now check the state variable, and handle the simple cases
	CcdState::State	s = exposureStatus();
	switch (s) {
		// not exposing, no need to wait, and no image is available
	case CcdState::idle:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "<-- LCK released wait()");
		return false;
	case CcdState::exposed:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "<-- LCK released wait()");
		return true;
	default:
		// remainder of cases is handled outside this switch
		break;
	}

	// case CcdState::exposing/cancelling
	std::string	ss = CcdState::state2string(s);
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"currently %s, waiting for operation to complete", ss.c_str());

	// Has the exposure time already expired? If so, we wait at
	// least as the exposure time indicates. We use the same timeout
	// for the wait to the cancellation operation, because some cameras
	// have no other way than to wait for the exposure to complete and
	// then to throw away the image.
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"lastexposurestart: %d, exposuretime: %f",
		lastexposurestart, exposure.exposuretime());
	double	endtime = lastexposurestart;
	endtime += exposure.exposuretime();
	endtime += 600; // additional time
	time_t	now = time(NULL);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "now: %d", now);
	int	delta = endtime - now;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "delta = %d", delta);

	// remember the current state
	CcdStateChange	statechange(this, s);

	// by waiting on the condition variable, the lock is released
	//debug(LOG_DEBUG, DEBUG_LOG, 0,
	//	"<-- LCK release lock for waiting wait()");

	// wait for a state change. Whenever the condition variable is
	// notified, check whether the state has changed, and retry if not
	while (std::cv_status::no_timeout == _condition.wait_for(lock,
		std::chrono::seconds(delta))) {
		// if we get to this point, we know that there was no timeout
		// and therefore that the state actually changed, and we
		// do own the lock
		debug(LOG_DEBUG, DEBUG_LOG, 0, "state change detected");
		if (ccd_lck_debug)
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"--> LCK wait complete, state %s",
				CcdState::state2string(state()).c_str());

		// if we get the exposed state, we return true
		if (CcdState::exposed == this->state()) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "state now exposed");
			return true;
		}

		// if we get any other state and not exposing, we return false
		if (CcdState::exposing != this->state()) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"state %s, giving up",
			CcdState::state2string(this->state()).c_str());
			return false;
		}

		// if we are still exposing, we we just continue waiting
		debug(LOG_DEBUG, DEBUG_LOG, 0, "still exposing, wait longer");
	}

	// this really should not happen, it indicates a serious problem
	// with the camera. Should we rather throw an exception here?
	debug(LOG_ERR, DEBUG_LOG, 0, "state change has timed out");
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "<-- LCK lock released wait()");
	return false;
}

/**
 * \brief Retrieve a raw image from the camera
 */
astro::image::ImagePtr	Ccd::getRawImage() {
	throw NotImplemented("getImage not implemented");
}

/**
 * \brief Retrieve an image
 *
 * This is the common driver method, it calls the raw image retrieval
 * function of the derived class, and if it gets an image back, it adds
 * the common metadata.
 */
astro::image::ImagePtr	Ccd::getImage() {
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "LCK retrieving raw image getImage()");
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "--> LCK acquired getImage()");
	// must have an exposed image to call this method
	CcdState::State	s = this->exposureStatus();
	if (CcdState::exposed != s) {
		std::string	msg = stringprintf("no exposed image to "
			"retrieve, bad state: %s",
			CcdState::state2string(s).c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw BadState(msg);
	}
	ImagePtr	image = this->getRawImage();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got a %d x %d image",
		image->size().width(), image->size().height());

	// add exposure meta data
	addMetadata(*image);

	// XXX if available, position information from the mount should
	//     also be added

	// set state to idle
	state(CcdState::idle);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "state changed to idle");

	//debug(LOG_DEBUG, DEBUG_LOG, 0, "<-- LCK release lock getImage()");

	// that's it, return the image
	return image;
}

/**
 * \brief Retrieve a sequence of images from the camera
 *
 * The default implementation just performs multiple startExposure/getImage
 * calls. We reuse the same exposure structure for all calls.
 * \param imagecount	number of images to retrieve
 */
astro::image::ImageSequence	Ccd::getImageSequence(unsigned int imagecount) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "getting image sequence of %d images",
		imagecount);
	astro::image::ImageSequence	result;
	unsigned int	k = 0;
	while (k < imagecount) {
		if (k > 0) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "start exposure %d", k);
			startExposure(exposure);
			usleep(1000000 * exposure.exposuretime());
		}
		wait();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "image complete");
		result.push_back(getImage());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "image %d retrieved", k);
		k++;
	}
	return result;
}

/**
 * \brief Start a stream with an ordinary camera
 */
void	Ccd::startStream(const Exposure& exposure) {
	CcdState::State	s = exposureStatus();
	if (CcdState::idle != exposureStatus()) {
		std::string	msg
			= stringprintf("cannot start stream in state %s",
				CcdState::state2string(s).c_str());
	}
	ImageStream::startStream(exposure);
}

/**
 * \brief Check whether we are currently streaming
 */
void	Ccd::checkStreaming() {
	if (!streaming()) {
		std::string	msg = stringprintf("not streaming, state %s",
			CcdState::state2string(exposureStatus()).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw BadState(msg);
	}
}

/**
 * \brief Stop a stream
 */
void	Ccd::stopStream() {
	checkStreaming();
	ImageStream::stopStream();
}

/**
 * \brief Change the stream exposure
 */
void	Ccd::streamExposure(const Exposure& exposure) {
	checkStreaming();
	ImageStream::streamExposure(exposure);
}

/**
 * \brief get the current stream exposure
 */
Exposure Ccd::streamExposure() {
	checkStreaming();
	return ImageStream::streamExposure();
}

/**
 * \brief Retrieve Cooler, using the cache if retrieved before
 */
CoolerPtr	Ccd::getCooler() {
	if (!cooler) {
		cooler = this->getCooler0();
	}
	return cooler;
}

/**
 * \brief Retrieve a cooler
 */
CoolerPtr	Ccd::getCooler0() {
	throw NotImplemented("thermoelectric cooler not implemented");
}

/**
 * \brief Retrieve the state of the shutter
 */
Shutter::state	Ccd::getShutterState() {
	throw NotImplemented("camera has no shutter");
}

/**
 * \brief Set the state of the shutter
 */
void	Ccd::setShutterState(const Shutter::state& state) {
	// always accept shutter open
	if (Shutter::OPEN == state) {
		return;
	}
	throw NotImplemented("camera has no shutter");
}

/**
 * \brief add exposure metadata
 */
void	Ccd::addExposureMetadata(ImageBase& image) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adding exposure metadata");
	exposure.addToImage(image);
	//std::cout << image;
}

/**
 * \brief add temperature metadata
 */
void	Ccd::addTemperatureMetadata(ImageBase& image) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adding temperature metadata");
	// only if a cooler is available for this CCD
	if (hasCooler()) {
		CoolerPtr	cooler = getCooler();
		cooler->addTemperatureMetadata(image);
	} else {
		if (hasTemperature()) {
			float	temperature = getTemperature()
					- Temperature::zero;
			image.setMetadata(FITSKeywords::meta(
				std::string("CCD-TEMP"), temperature));
		}
	}
}

/**
 * \brief add metadata
 */
void	Ccd::addMetadata(ImageBase& image) {
	info.addMetadata(image);
	this->addExposureMetadata(image);
	this->addTemperatureMetadata(image);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adding DATE-OBS and UUID");
	image.setMetadata(
		FITSKeywords::meta(std::string("DATE-OBS"),
			FITSdate()));
	image.setMetadata(FITSKeywords::meta(std::string("UUID"), UUID()));
	image.setMetadata(FITSKeywords::meta(std::string("CAMERA"),
		this->userFriendlyName()));
}

/**
 * \brief Get the interval of valid gain values
 */
std::pair<float, float>	Ccd::gainInterval() {
	return std::make_pair((float)1., (float)1.);
}

/**
 * \brief Initiate the callback to all the installed callback handlers
 *
 * \param state		the new ccd state to register
 */
void	Ccd::stateUpdate(const CcdState& state) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "propagating new state to callbacks");
	callback::CallbackDataPtr	cb(new CcdStateCallbackData(state));
	_callback(cb);
}

/**
 * \brief Add callback for state changes
 *
 * \param callback	the callback class to add
 */
void	Ccd::addCallback(callback::CallbackPtr callback) {
	_callback.insert(callback);
}

/**
 * \brief Remove callback for state changes
 *
 * \param callback	the callback class to remove
 */
void	Ccd::removeCallback(callback::CallbackPtr callback) {
	auto	i = _callback.find(callback);
	if (i != _callback.end()) {
		_callback.erase(i);
	}
}



} // namespace camera
} // namespace astro
