/*
 * AtikCcd.cpp -- ATIK CCD implementation
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AtikCcd.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroExceptions.h>

namespace astro {
namespace camera {
namespace atik {

AtikCcd::AtikCcd(CcdInfo& info, ::AtikCamera *camera)
	: Ccd(info), _camera(camera) {
        // get the capabilities
	const char	*name;
	CAMERA_TYPE	type;
	_camera->getCapabilities(&name, &type, &capa);
}

AtikCcd::~AtikCcd() {
}

void	AtikCcd::run() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "run method of AtikCcd starting");
	_image = ImagePtr(NULL);
	bool	rc;
	if (capa.hasShutter) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "setting shutter to %s",
			(exposure.needsshutteropen()) ? "open" : "closed");
		rc = _camera->setShutter(exposure.needsshutteropen());
	}
	rc = _camera->startExposure(true);
	if (!rc) {
		// OK, we assume that we cannot to a long exposure, try a
		// short one
		rc = _camera->readCCD(exposure.x(), exposure.y(),
			exposure.width(), exposure.height(),
			exposure.mode().x(), exposure.mode().y(),
			exposure.exposuretime());
		if (!rc) {
			state(CcdState::exposed);
			std::string	msg = stringprintf("cannot do "
				"exposure %.3f sec", exposure.exposuretime());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
	} else {
		// wait for the exposure time to expire
		usleep(_camera->delay(exposure.exposuretime()));
		rc = _camera->readCCD(exposure.x(), exposure.y(),
			exposure.width(), exposure.height(),
			exposure.mode().x(), exposure.mode().y());
		if (!rc) {
			state(CcdState::exposed);
			std::string	msg = stringprintf("cannot read after long exposure");
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
	}
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
		std::string	msg = stringprintf("cannot read data");
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
