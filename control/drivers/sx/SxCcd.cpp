/*
 * SxCcd.cpp -- Starlight express CCD implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SxCcd.h>
#include <AstroCamera.h>
#include <AstroImage.h>
#include <sx.h>
#include <debug.h>
#include <SxUtils.h>

using namespace astro::camera;

namespace astro {
namespace camera {
namespace sx {

SxCcd::SxCcd(const CcdInfo& info, SxCamera& _camera, int _ccdindex)
	: Ccd(info), camera(_camera), ccdindex(_ccdindex) {
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
	if (0 != ccdindex) {
		throw std::runtime_error("only imaging CCD has cooler");
	}
	return CoolerPtr(new SxCooler(camera));
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
	rpd.x_bin = exposure.mode.getY();
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
ShortImagePtr	SxCcd::shortImage() throw (not_implemented) {
	// compute the size of the buffer, and create a buffer for the
	// data
	int	size = 2 * exposure.frame.size.pixels;
	unsigned short	*data = new unsigned short[size];

	// read the data from the data endpoint
	BulkTransfer	transfer(camera.getEndpoint(), size, (unsigned char *)data);

	// submit the transfer
	camera.getDevicePtr()->submit(&transfer);

	// now the camera is no longer busy, i.e. we have to reset the state
	state = Exposure::idle;

	// when the transfer completes, one can use the data for the
	// image
	Image<unsigned short>	*image
		= new Image<unsigned short>(exposure.frame.size, data);
	return ShortImagePtr(image);
}

} // namespace sx
} // namespace camera
} // namespace astro
