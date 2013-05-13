/*
 * SxCcd.cpp -- Starlight express CCD implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SxCcd.h>
#include <AstroCamera.h>
#include <AstroImage.h>
#include <AstroFilter.h>
#include <sx.h>
#include <debug.h>
#include <SxUtils.h>

using namespace astro::camera;
using namespace astro::image::filter;

namespace astro {
namespace camera {
namespace sx {

SxCcd::SxCcd(const CcdInfo& info, SxCamera& _camera, int _ccdindex)
	: Ccd(info), camera(_camera), ccdindex(_ccdindex) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating CCD %d", ccdindex);
}

SxCcd::~SxCcd() {
}

/**
 * \brief Get the thermoelectric cooler
 *
 * This method builds a SxCooler objects and wraps in a CoolerPtr smart
 * pointer.
 */
CoolerPtr	SxCcd::getCooler() throw (not_implemented) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "request for cooler");
	try {
		return camera.getCooler(ccdindex);
	} catch (std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cooler problem: %s", x.what());
		throw not_implemented("no cooler");
	}
}

/**
 * \brief get the exposure status.
 *
 * This method does not query the actual camera status, but only the current
 * flag. So it may return a busy state although the exposure has long been
 * completed.
 */
Exposure::State	SxCcd::exposureStatus() throw (not_implemented) {
	return state;
}

/**
 * \brief Start an Exposure on a "normal" Starlight Express camera
 *
 * \param exposure	specification of the exposure to take
 */
void	SxCcd::startExposure(const Exposure& exposure) throw (not_implemented) {
	// remember the exposure
	this->exposure = exposure;

	// we should check that the selected binning mode is in fact 
	// available
	if (!info.modes().permits(exposure.mode)) {
		debug(LOG_ERR, DEBUG_LOG, 0, "binning mode %s not supported",
			exposure.mode.toString().c_str());
		throw SxError("binning mode not supported");
	}

	// create the exposure request
	sx_read_pixels_delayed_t	rpd;
	rpd.x_offset = exposure.frame.origin.x;
	rpd.y_offset = exposure.frame.origin.y;
	rpd.width = exposure.frame.size.width;
	rpd.height = exposure.frame.size.height;
	rpd.x_bin = exposure.mode.getX();
	rpd.y_bin = exposure.mode.getY();
	rpd.delay = 1000 * exposure.exposuretime;

	// build a control request
	Request<sx_read_pixels_delayed_t>	request(
		RequestBase::vendor_specific_type,
		RequestBase::device_recipient, ccdindex,
		(uint8_t)SX_CMD_READ_PIXELS_DELAYED, (uint16_t)0, &rpd);
	camera.controlRequest(&request);

	// we are now in exposing state
	state = Exposure::exposing;
}

/**
 * \brief Retrieve an image with short pixel values.
 *
 * Starlight Express cameras always use 16 bit pixels, it is natural to
 * always produce 16 bit deep images.
 */
ImagePtr	SxCcd::getImage() throw (not_implemented) {
	// compute the target image size, using the binning mode
	ImageSize	targetsize(
		exposure.frame.size.width / exposure.mode.getX(),
		exposure.frame.size.height / exposure.mode.getY());

	// compute the size of the buffer, and create a buffer for the data
	int	size = targetsize.pixels;
	unsigned short	*data = new unsigned short[size];

	// read the data from the data endpoint
	BulkTransfer	transfer(camera.getEndpoint(),
		sizeof(unsigned short) * size, (unsigned char *)data);

	// timeout depends on the actual data size we want to transfer
	int	timeout = 1100 * exposure.exposuretime + 30000;
	transfer.setTimeout(timeout);

	// submit the transfer
	camera.getDevicePtr()->submit(&transfer);

	// now the camera is no longer busy, i.e. we have to reset the state
	state = Exposure::idle;

	// when the transfer completes, one can use the data for the image
	Image<unsigned short>	*image
		= new Image<unsigned short>(targetsize, data);

	// images are upside down, since our origin is always the lower
	// left corner
	FlipOperator<unsigned short>	f;
	f(*image);

	// if the exposure requests a limiting function, we apply it now
	if (exposure.limit < INFINITY) {
		for (unsigned int offset = 0; offset < image->size.pixels;
			offset++) {
			unsigned short	pv = image->pixels[offset];
			if (pv > exposure.limit) {
				pv = exposure.limit;
			}
			image->pixels[offset] = pv;
		}
	}

	return ImagePtr(image);
}

} // namespace sx
} // namespace camera
} // namespace astro
