/*
 * CcdI.cpp -- ICE CCD wrapper implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <CcdI.h>
#include <CoolerI.h>
#include <NameConverter.h>
#include <AstroExceptions.h>
#include <ImageI.h>
#include <ImagesI.h>
#include <ProxyCreator.h>
#include <AstroExceptions.h>
#include <IceConversions.h>
#include <Ice/Connection.h>
#include <ImageDirectory.h>
#include "StatisticsI.h"

namespace snowstar {

/**
 * \brief Construct a Ccd server wrapper
 *
 * \param ccd	the ccd this servant is supposed to represent
 */
CcdI::CcdI(astro::camera::CcdPtr ccd) : DeviceI(*ccd), _ccd(ccd) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create the ccd callback");
	CcdICallback	*ccdcallback = new CcdICallback(*this);
	ccdcallbackptr = CcdICallbackPtr(ccdcallback);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "install the callback in the ccd");
	_ccd->addCallback(ccdcallbackptr);
}

/**
 * \brief Destroy the Ccd server wrapper
 */
CcdI::~CcdI() {
	_ccd->removeCallback(ccdcallbackptr);
}

/**
 * \brief return the Ccd information
 *
 * \param current	the current call context
 */
CcdInfo	CcdI::getInfo(const Ice::Current& current) {
	CallStatistics::count(current);
	return convert(_ccd->getInfo());
}

/**
 * \brief return the Exposue status
 *
 * \param current	the current call context
 */
ExposureState	CcdI::exposureStatus(const Ice::Current& current) {
	CallStatistics::count(current);
	if (_ccd->streaming()) {
		return snowstar::STREAMING;
	}
	// this method may not throw exceptions, so we need a state
	// to indicate that the camera is broken, something we don't
	// have in the original astro::camera:Ccd class
	try {
		return convert(_ccd->exposureStatus());
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "CCD is in unknown state: %s",
			x.what());
	}
	return snowstar::BROKEN;
}

/**
 * \brief Start a new exposure
 *
 * \param exposure	the exposure data tostart the exposure
 * \param current	the current call context
 */
void	CcdI::startExposure(const Exposure& exposure,
		const Ice::Current& current) {
	CallStatistics::count(current);
	if (_ccd->streaming()) {
		throw BadState("cannot start exposure while streaming");
	}
	image.reset();
	try {
		_ccd->startExposure(convert(exposure));
	} catch (astro::BadParameter& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "bad parameter: %s", x.what());
		BadParameter	y;
		y.cause = x.what();
		throw y;
	} catch (astro::DeviceException& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "startExposure error: %s",
			x.what());
		DeviceException	y;
		y.cause = x.what();
		throw y;
	}
	laststart = time(NULL);
}

/**
 * \brief Return the time when the last exposure was started
 *
 * \param current	the current call context
 */
int	CcdI::lastExposureStart(const Ice::Current& current) {
	CallStatistics::count(current);
	return laststart;
}

/**
 * \brief Cancel an exposure
 *
 * \param current	the current call context
 */
void	CcdI::cancelExposure(const Ice::Current& current) {
	CallStatistics::count(current);
	if (_ccd->streaming()) {
		throw BadState("cannot cancel exposure while streaming");
	}
	try {
		_ccd->cancelExposure();
	} catch (const astro::camera::BadState& badstate) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cancelExposure bad state: %s",
			badstate.what());
		throw BadState(badstate.what());
	} catch (const astro::DeviceException& deviceexception) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cancelExposure error: %s",
			deviceexception.what());
		throw DeviceException(deviceexception.what());
	}
}

/**
 * \brief Get the exposure data in use for the current/last exposure
 *
 * \param current	the current call context
 */
Exposure	CcdI::getExposure(const Ice::Current& current) {
	CallStatistics::count(current);
	try {
		return convert(_ccd->getExposure());
	} catch (const astro::camera::BadState& badstate) {
		debug(LOG_ERR, DEBUG_LOG, 0, "getExposure bad state: %s",
			badstate.what());
		throw BadState(badstate.what());
	} catch (const astro::DeviceException& deviceexception) {
		debug(LOG_ERR, DEBUG_LOG, 0, "getExposure error: %s",
			deviceexception.what());
		throw DeviceException(deviceexception.what());
	}
}

/**
 * \brief Get an image proxy to retrieve an image
 *
 * \param current	the current call context
 */
ImagePrx	CcdI::getImage(const Ice::Current& current) {
	CallStatistics::count(current);
	if (_ccd->streaming()) {
		throw BadState("cannot get image while streaming");
	}
	// get the image and add it to the ImageDirectory
	if (!image) {
		try {
			image = _ccd->getImage();
		} catch (astro::camera::BadState& bsx) {
			debug(LOG_ERR, DEBUG_LOG, 0,
				"bad state in getImage: %s",
				bsx.what());
			throw BadState("no image");
		} catch (const astro::DeviceException& deviceexception) {
			debug(LOG_ERR, DEBUG_LOG, 0,
				"device exception in getImage: %s",
				deviceexception.what());
			throw DeviceException(deviceexception.what());
		}
	}

	// save image
	ImageDirectory	_imagedirectory;
	std::string	filename = _imagedirectory.save(image);
	
	return snowstar::getImage(filename, current);
}

/**
 * \brief Check whether the ccd has a gain setting
 *
 * \param current	the current call context
 */
bool	CcdI::hasGain(const Ice::Current& current) {
	CallStatistics::count(current);
	return _ccd->hasGain();
}

/**
 * \brief Retrieve the gain of the ccd
 *
 * \param current	the current call context
 */
float	CcdI::getGain(const Ice::Current& current) {
	CallStatistics::count(current);
	return _ccd->getGain();
}

/**
 * \brief Get the interval of valid gains
 *
 * \param current	the current call context
 */
Interval	CcdI::gainInterval(const Ice::Current& current) {
	CallStatistics::count(current);
	return convert(_ccd->gainInterval());
}

/**
 * \brief Check whether the camera has a shutter
 *
 * \param current	the current call context
 */
bool	CcdI::hasShutter(const Ice::Current& current) {
	CallStatistics::count(current);
	return _ccd->hasShutter();
}

/**
 * \brief Set the shutter state
 *
 * \param current	the current call context
 */
ShutterState	CcdI::getShutterState(const Ice::Current& current) {
	CallStatistics::count(current);
	return convert(_ccd->getShutterState());
}

/**
 * \brief Set the shutter state
 *
 * \param state		the shutter state
 * \param current	the current call context
 */
void	CcdI::setShutterState(ShutterState state,
		const Ice::Current& current) {
	CallStatistics::count(current);
	_ccd->setShutterState(convert(state));
}

/**
 * \brief Check whether the camera has a cooler
 *
 * \param current	the current call context
 */
bool	CcdI::hasCooler(const Ice::Current& current) {
	CallStatistics::count(current);
	return _ccd->hasCooler();
}

typedef IceUtil::Handle<CoolerI>	CoolerIPtr;

/**
 * \brief Get the cooler
 *
 * \param current	the current call context
 */
CoolerPrx	CcdI::getCooler(const Ice::Current& current) {
	CallStatistics::count(current);
	std::string	name = _ccd->getCooler()->name();
	return snowstar::createProxy<CoolerPrx>(name, current);
}

/**
 * \brief Create a proxy for a given ccd name
 *
 * \param ccdname	the name of the ccd to return
 * \param current	the current call context
 */
CcdPrx	CcdI::createProxy(const std::string& ccdname,
		const Ice::Current& current) {
	CallStatistics::count(current);
	return snowstar::createProxy<CcdPrx>(ccdname, current);
}

/**
 * \brief Register a servant that acts as an image sink for a stream
 *
 * \param imagesinkidentity	id if the image sink
 * \param current		the current call context
 */
void	CcdI::registerSink(const Ice::Identity& imagesinkidentity,
		const Ice::Current& current) {
	CallStatistics::count(current);
	if (_sink) {
		try {
			_sink->stop();
		} catch (const std::exception& x) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "sink failed: %s",
				x.what());
			_sink = NULL;
		}
	}
	CcdSink	*sink = new CcdSink(_ccd, imagesinkidentity, current);
	_ccd->imagesink(sink);
	_sink = CcdSinkPtr(sink);
}

/**
 * \brief Start the stream
 *
 * \param e		the exposure settings to use for the stream
 * \param current	the current call context
 */
void	CcdI::startStream(const ::snowstar::Exposure& e,
		const Ice::Current& current) {
	CallStatistics::count(current);
	if (_ccd->streaming()) {
		throw BadState("already streaming");
	}
	if (!_sink) {
		throw BadState("no registered images ink");
	}
	_ccd->startStream(convert(e));
}

/**
 * \brief Update the stream
 *
 * \param e		the new exposure settings
 * \param current	the current call context
 */
void	CcdI::updateStream(const ::snowstar::Exposure& e,
		const Ice::Current& current) {
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new exposure time: %.1f",
		e.exposuretime);
	_ccd->streamExposure(convert(e));
}

/**
 * \brief Stop the stream
 *
 * \param current	the current call context
 */
void	CcdI::stopStream(const ::Ice::Current& current) {
	CallStatistics::count(current);
	if (!_ccd->streaming()) {
		throw BadState("cannot stop stream: not streaming");
	}
	_ccd->stopStream();
	if (_sink) {
		try {
			_sink->stop();
		} catch (const std::exception& x) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "sink failed: %s",
				x.what());
		}
	}
	_sink = NULL;
}

/**
 * \brief Unregister the stream image sink
 *
 * \param current	the current call context
 */
void	CcdI::unregisterSink(const ::Ice::Current& current) {
	CallStatistics::count(current);
	stopStream(current);
	_sink = NULL;
}

/**
 * \brief Register a callback for state upates
 *
 * \param callback	the callback id to register
 * \param current	the current call context
 */
void	CcdI::registerCallback(const Ice::Identity& callback,
		const Ice::Current& current) {
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "register %s",
			callback.name.c_str());
		callbacks.registerCallback(callback, current);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot register callback %s: %s",
			astro::demangle_cstr(x), x.what());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"cannot register callback %s, unknown reason",
			callback.name.c_str());
	}
}

/**
 * \brief Unregister a callback for state upates
 *
 * \param callback	the callback id to unregister
 * \param current	the current call context
 */
void	CcdI::unregisterCallback(const Ice::Identity& callback,
		const Ice::Current& current) {
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "unregister %s",
			callback.name.c_str());
		callbacks.unregisterCallback(callback, current);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot unregister callback %s: %s",
			astro::demangle_cstr(x), x.what());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"cannot unregister callback %s, unknown reason",
			callback.name.c_str());
	}
}

/**
 * \brief Method to forward the state change to the callbacks
 *
 * \param data	the callback data
 */
void	CcdI::stateUpdate(const astro::callback::CallbackDataPtr data) {
	try {
		callbacks(data);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot send callbacks: %s",
			x.what());
		throw x;
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot send callbacks (unknown)");
	}
}

/**
 * \brief Adapter method for the camera state callback
 *
 * \param p		the callback proxy to call
 * \param data		the callback data to use as argument in the call
 */
template<>
void	callback_adapter<CcdCallbackPrx>(CcdCallbackPrx& p,
		const astro::callback::CallbackDataPtr data) {
	astro::camera::CcdStateCallbackData	*cs
		= dynamic_cast<astro::camera::CcdStateCallbackData*>(&*data);
	if (NULL != cs) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "state callback");
		snowstar::ExposureState	s = convert(cs->data());
		p->state(s);
		return;
	}
}

} // namespace snowstar
