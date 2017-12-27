/*
 * AtikCcd.cpp -- ATIK CCD implementation
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AtikCcd.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroExceptions.h>
#include <AtikCooler.h>
#include <includes.h>

namespace astro {
namespace camera {
namespace atik {

/**
 * \brief Construct the CCD for an Atik-Camera
 */
AtikCcd::AtikCcd(CcdInfo& info, astro::camera::atik::AtikCamera& camera)
	: Ccd(info), _camera(camera) {
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start run method for atikccd");
	_camera.exposureRun(Ccd::exposure, *this);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "run method for atikccd complete");
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
	_camera.abortExposure();
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
	return _camera.capa().hasShutter;
}

bool	AtikCcd::hasCooler() const {
	return (_camera.capa().cooler != COOLER_NONE);
}

CoolerPtr	AtikCcd::getCooler0() {
	return CoolerPtr(new AtikCooler(_camera));
}

void	AtikCcd::image(ImagePtr image) {
	_image = image;
}

} // namespace atik
} // namespace camera
} // namespace astro
