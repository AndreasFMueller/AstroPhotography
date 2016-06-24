/*
 * QhyCcd.cpp -- implementation of a QHY ccd object
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <QhyCcd.h>
#include <AstroDebug.h>
#include <AstroUtils.h>
#include <AstroExceptions.h>
#include <QhyCooler.h>

namespace astro {
namespace camera {
namespace qhy {

/**
 * \brief Construct an QHY CCD object
 */
QhyCcd::QhyCcd(const CcdInfo& info, const ::qhy::DevicePtr devptr,
	QhyCamera& _camera)
	: Ccd(info), deviceptr(devptr), camera(_camera) {
}

/**
 * \brief Destroy the QHY CCD object
 */
QhyCcd::~QhyCcd() {
}

/**
 * \brief main function for the thread
 */
void	start_routine(QhyCcd *ccd) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start exposure thread");
	ccd->getImage0();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "end exposure thread");
}

/**
 * \brief Start an exposure
 */
void	QhyCcd::startExposure(const Exposure& exposure) {
	Ccd::startExposure(exposure);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "launch a new thread");
	thread = std::thread(start_routine, this);
}

/**
 * \brief class specific image retrieval from the QHY camera
 */
void	QhyCcd::getImage0() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "starting getImage0()");
	state = CcdState::exposing;
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

	// that's it
	state = CcdState::exposed;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "getImage0() complete");
}

/**
 * \brief collect the image when exposure is done
 */
ImagePtr	QhyCcd::getRawImage() {
	if (state != CcdState::exposed) {
		throw BadState("no exposure available");
	}
	thread.join();
	state = CcdState::idle;
	return image;
}

/**
 * \brief construct a cooler
 */
CoolerPtr	QhyCcd::getCooler0() {
	return CoolerPtr(new QhyCooler(camera, deviceptr));
}

} // namespace qhy
} // namespace camery
} // namespace astro
