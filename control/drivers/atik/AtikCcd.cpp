/*
 * AtikCcd.cpp -- ATIK CCD implementation
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AtikCcd.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroExceptions.h>
#include <includes.h>

namespace astro {
namespace camera {
namespace atik {

/**
 * \brief Construct the CCD for an Atik-Camera
 */
AtikCcd::AtikCcd(CcdInfo& info, ::AtikCamera *camera)
	: Ccd(info), _camera(camera) {
        // get the capabilities
	const char	*name;
	CAMERA_TYPE	type;
	_camera->getCapabilities(&name, &type, &capa);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "created AtikCcd %s, %s", name,
		(type == ORIGINAL_HSC) ? "HSC" : (
		(type == IC24) ? "IC24" : (
		(type == QUICKER) ? "QUICKER" : (
		(type == IIDC) ? "SONY_SCI" : "(unknown)"))));
}

/** 
 * \brief destroy the atik camera
 */
AtikCcd::~AtikCcd() {
}

/**
 * \brief Private run method that performs the actual exposure on an ATIK camera
 */
void	AtikCcd::run() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "run method of AtikCcd starting");
	_image = ImagePtr(NULL);
	bool	rc;

	// set the shutter if we have
	if (capa.hasShutter) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "setting shutter to %s",
			(exposure.needsshutteropen()) ? "open" : "closed");
		rc = _camera->setShutter(exposure.needsshutteropen());
		if (!rc) {
			std::string	msg = stringprintf("cannot set shutter:"
				" %s", _camera->getLastError());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s");
			throw std::runtime_error(msg);
		}
	}

	// now we have to decide whether we can do a long exposure
	if (exposure.exposuretime() < capa.maxShortExposure) {
		// do a short exposure
		rc = _camera->readCCD(exposure.x(), exposure.y(),
			exposure.width(), exposure.height(),
			exposure.mode().x(), exposure.mode().y(),
			exposure.exposuretime());
		if (!rc) {
			state(CcdState::exposed);
			std::string	msg = stringprintf("cannot do "
				"exposure %.3f sec: %s",
				exposure.exposuretime(),
				_camera->getLastError());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
	} else {
		if (!capa.supportsLongExposure) {
			std::string	msg = stringprintf("camera does not "
				"support long exposure, and exposure time %.3f "
				"exceeds limit %.3f for short exposures",
				exposure.exposuretime(), capa.maxShortExposure);
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
		rc = _camera->startExposure(true);
		if (!rc) {
			// cannot start exposure
			std::string	msg = stringprintf("cannot start long "
				"exposure: %s", _camera->getLastError());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
		// wait for the exposure time to expire
		usleep(_camera->delay(exposure.exposuretime()));
		rc = _camera->readCCD(exposure.x(), exposure.y(),
			exposure.width(), exposure.height(),
			exposure.mode().x(), exposure.mode().y());
		if (!rc) {
			state(CcdState::exposed);
			std::string	msg = stringprintf("cannot read after "
				"long exposure: %s", _camera->getLastError());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
	}

	// interpreting the data we have received
	debug(LOG_DEBUG, DEBUG_LOG, 0, "exposure %s complete. reading rata",
		exposure.toString().c_str());
	ImageSize	size = exposure.frame().size() / exposure.mode();
	Image<unsigned short>	*image = new Image<unsigned short>(size);
	image->setOrigin(exposure.frame().origin());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "reading image data: %d pixels",
		image->getSize().getPixels());
	rc = _camera->getImage(image->pixels, image->getSize().getPixels());
	if (!rc) {
		state(CcdState::exposed);
		std::string	msg = stringprintf("cannot read data: %s",
			_camera->getLastError());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "reading image data complete");
	_image = ImagePtr(image);
	state(CcdState::exposed);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "run method of AtikCcd complete");
}

static void	main(AtikCcd *atikccd) {
	try {
		atikccd->run();
	} catch (std::exception& ex) {
		std::string	msg = stringprintf("atik exposure thread "
			"terminated by %s: %s",
			demangle(typeid(ex).name()).c_str(), ex.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	} catch (...) {
		std::string	msg("atik exposure thread terminated by "
					"unknown exception");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	}
}

void	AtikCcd::startExposure(const Exposure& exposure) {
	Ccd::startExposure(exposure);
	state(CcdState::exposing);
	_thread = std::shared_ptr<std::thread>(new std::thread(main, this));
}

void	AtikCcd::cancelExposure() {
	_camera->abortExposure();
	throw std::runtime_error("cancelExposure not implemented yet");
}

ImagePtr	AtikCcd::getRawImage() {
	if (state() != CcdState::exposed) {
		throw BadState("no exposure available");
	}
	_thread->join();
	if (!_image) {
		throw BadState("no image: exposure failed");
	}
	state(CcdState::idle);
	return _image;
}

bool	AtikCcd::hasShutter() const {
	return capa.hasShutter;
}

bool	AtikCcd::hasCooler() const {
	return (capa.cooler != COOLER_NONE);
}

CoolerPtr	AtikCcd::getCooler0() {
	return CoolerPtr(NULL);
}

} // namespace atik
} // namespace camera
} // namespace astro
