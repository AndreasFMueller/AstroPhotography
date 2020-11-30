/*
 * Qhy2Ccd.cpp -- implementation of a QHY ccd object
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Qhy2Ccd.h>
#include <AstroDebug.h>
#include <AstroUtils.h>
#include <AstroExceptions.h>
#include <Qhy2Cooler.h>

namespace astro {
namespace camera {
namespace qhy2 {

/**
 * \brief Construct an QHY CCD object
 */
Qhy2Ccd::Qhy2Ccd(const CcdInfo& info, Qhy2Camera& _camera)
	: Ccd(info), camera(_camera) {
}

/**
 * \brief Destroy the QHY CCD object
 */
Qhy2Ccd::~Qhy2Ccd() {
	if (thread.joinable()) {
		thread.join();
	}
}

/**
 * \brief main function for the thread
 */
void	Qhy2Ccd::main(Qhy2Ccd *ccd) noexcept {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start exposure thread");
	try {
		ccd->getImage0();
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot expose: %s", x.what());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot expose");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "end exposure thread");
}

/**
 * \brief Start an exposure
 */
void	Qhy2Ccd::startExposure(const Exposure& exposure) {
	Ccd::startExposure(exposure);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "launch a new thread");
	thread = std::thread(main, this);
}

/**
 * \brief class specific image retrieval from the QHY camera
 */
void	Qhy2Ccd::getImage0() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "starting getImage0()");
#if 0
	state(CcdState::exposing);
	this->exposure = exposure;
	::qhy::BinningMode	mode(exposure.mode().x(), exposure.mode().y());
	deviceptr->camera().mode(mode);
	deviceptr->camera().exposuretime(exposure.exposuretime());
	deviceptr->camera().startExposure();

	// get the image and convert to an image
	::qhy::ImageBufferPtr	buffer = deviceptr->camera().getImage()
						->active_buffer();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got image of size %dx%d",
		buffer->width(), buffer->height());
	Image<unsigned short>	*imagecontent
		= new Image<unsigned short>(buffer->width(), buffer->height());
	image = ImagePtr(imagecontent);
	for (unsigned int x = 0; x < buffer->width(); x++) {
		for (unsigned int y = 0; y < buffer->height(); y++) {
			imagecontent->pixel(x, y) = buffer->p(x, y);
		}
	}

	// if the camera is a color camera, add the bayer type to the image
	std::string	b = deviceptr->camera().bayer();
	if (b.size() > 0) {
		image->setMosaicType(MosaicType(b));
	}
#endif

	// that's it
	state(CcdState::exposed);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "getImage0() complete");
}

/**
 * \brief collect the image when exposure is done
 */
ImagePtr	Qhy2Ccd::getRawImage() {
	if (state() != CcdState::exposed) {
		throw BadState("no exposure available");
	}
	thread.join();
	state(CcdState::idle);
	return image;
}

/**
 * \brief construct a cooler
 */
CoolerPtr	Qhy2Ccd::getCooler0() {
	return CoolerPtr(new Qhy2Cooler(camera));
}

} // namespace qhy2
} // namespace camery
} // namespace astro
