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


namespace snowstar {

/**
 * \brief Construct a Ccd server wrapper
 */
CcdI::CcdI(astro::camera::CcdPtr ccd) : DeviceI(*ccd), _ccd(ccd) {
}

/**
 * \brief Destroy the Ccd server wrapper
 */
CcdI::~CcdI() {
}

/**
 * \brief return the Ccd information
 */
CcdInfo	CcdI::getInfo(const Ice::Current& /* current */) {
	return convert(_ccd->getInfo());
}

/**
 * \brief return the Exposue status
 */
ExposureState	CcdI::exposureStatus(const Ice::Current& /* current */) {
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
 * \param start an exposue
 */
void	CcdI::startExposure(const Exposure& exposure,
		const Ice::Current& /* current */) {
	if (_ccd->streaming()) {
		throw BadState("cannot start exposure while streaming");
	}
	image.reset();
	try {
		_ccd->startExposure(convert(exposure));
	} catch (astro::BadParameter& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "bad parameter: %s", x.what());
		throw BadParameter(x.what());
	} catch (astro::DeviceException& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "startExposure error: %s",
			x.what());
		throw DeviceException(x.what());
	}
	laststart = time(NULL);
}

/**
 * \brief Return the time when the last exposure was started
 */
int	CcdI::lastExposureStart(const Ice::Current& /* current */) {
	return laststart;
}

/**
 * \brief Cancel an exposure
 */
void	CcdI::cancelExposure(const Ice::Current& /* current */) {
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
 */
Exposure	CcdI::getExposure(const Ice::Current& /* current */) {
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
 */
ImagePrx	CcdI::getImage(const Ice::Current& current) {
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

bool	CcdI::hasGain(const Ice::Current& /* current */) {
	return _ccd->hasGain();
}

Interval	CcdI::gainInterval(const Ice::Current& /* current */) {
	return convert(_ccd->gainInterval());
}

bool	CcdI::hasShutter(const Ice::Current& /* current */) {
	return _ccd->hasShutter();
}

ShutterState	CcdI::getShutterState(const Ice::Current& /* current */) {
	return convert(_ccd->getShutterState());
}

void	CcdI::setShutterState(ShutterState state,
		const Ice::Current& /* current */) {
	_ccd->setShutterState(convert(state));
}

bool	CcdI::hasCooler(const Ice::Current& /* current */) {
	return _ccd->hasCooler();
}

typedef IceUtil::Handle<CoolerI>	CoolerIPtr;

CoolerPrx	CcdI::getCooler(const Ice::Current& current) {
	std::string	name = _ccd->getCooler()->name();
	return snowstar::createProxy<CoolerPrx>(name, current);
}

CcdPrx	CcdI::createProxy(const std::string& ccdname,
		const Ice::Current& current) {
	return snowstar::createProxy<CcdPrx>(ccdname, current);
}

void	CcdI::registerSink(const Ice::Identity& imagesinkidentity,
		const Ice::Current& current) {
	if (_sink) {
		try {
			_sink->stop();
		} catch (const std::exception& x) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "sink failed: %s",
				x.what());
			_sink = NULL;
		}
	}
	CcdSink	*sink = new CcdSink(imagesinkidentity, current);
	_ccd->imagesink(sink);
	_sink = CcdSinkPtr(sink);
}

void	CcdI::startStream(const ::snowstar::Exposure& e,
		const Ice::Current& /* current */) {
	if (_ccd->streaming()) {
		throw BadState("already streaming");
	}
	if (!_sink) {
		throw BadState("no registered images ink");
	}
	_ccd->startStream(convert(e));
}

void	CcdI::updateStream(const ::snowstar::Exposure& e,
		const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new exposure time: %.1f",
		e.exposuretime);
	_ccd->streamExposure(convert(e));
}

void	CcdI::stopStream(const ::Ice::Current& /* current */) {
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

void	CcdI::unregisterSink(const ::Ice::Current& current) {
	stopStream(current);
	_sink = NULL;
}

} // namespace snowstar
