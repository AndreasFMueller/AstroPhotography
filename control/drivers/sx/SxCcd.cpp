/*
 * SxCcd.cpp -- Starlight express CCD implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "SxCcd.h"
#include <AstroCamera.h>
#include <AstroImage.h>
#include <AstroFilter.h>
#include <AstroOperators.h>
#include <AstroExceptions.h>
#include "sx.h"
#include <AstroDebug.h>
#include <AstroUtils.h>
#include "SxUtils.h"
#include "SxCooler.h"

using namespace astro::camera;
using namespace astro::image::filter;
using namespace astro::image::operators;

namespace astro {
namespace camera {
namespace sx {

/**
 * \brief Construct an SxCcd
 *
 * \param info		the ccdinfo for which to construct the ccd
 * \param _camera	the camera this ccd is a part of
 * \param _ccdindex	the index of the ccd (in the ccdinfo)
 */
SxCcd::SxCcd(const CcdInfo& info, SxCamera& _camera, int _ccdindex)
	: Ccd(info), camera(_camera), ccdindex(_ccdindex) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating CCD %d", ccdindex);
}

/**
 * \brief Destroy an SxCcd
 */
SxCcd::~SxCcd() {
	// XXX here we should really destroy the thread
}

/**
 * \brief Start Routine of the exposure thread
 *
 * \param ccd	the SxCcd to use for the exposure
 */
void	start_routine(SxCcd *ccd) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start exposure thread");
#if 0
	pthread_attr_t	attr;
	pthread_attr_init(&attr);
	size_t	stacksize = 0;
	pthread_attr_getstacksize(&attr, &stacksize);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stack size: %lu", stacksize);
#endif

	try {
		ccd->getImage0();
	} catch (USBError& x) {
		// construct an error message
		std::string	msg = stringprintf("getImage0 failed: %s %s",
			demangle(typeid(x).name()).c_str(), x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());

		// refresh the connection
		ccd->refresh();
	} catch (DeviceTimeout& x) {
		// construct an error message
		std::string	msg = stringprintf("getImage0 failed: %s %s",
			demangle(typeid(x).name()).c_str(), x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());

		// refresh the connection
		ccd->refresh();
	} catch (const std::exception& x) {
		// construct an error message
		std::string	msg = stringprintf("getImage0 failed: %s %s",
			demangle(typeid(x).name()).c_str(), x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "end exposure thread");
}

/**
 * \brief Start the exposure
 *
 * This method calls the "real" startExposure0 method and launches a thread
 * that performs the getImage0 method which will retrieve the image. 
 */
void	SxCcd::startExposure(const Exposure& exposure) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "SxCcd::startExposure called");
	Ccd::startExposure(exposure);

	// create a new thread
	debug(LOG_DEBUG, DEBUG_LOG, 0, "launch a new thread");
	thread = std::thread(start_routine, this);
}

/**
 * \brief Get the exposed image
 *
 * This method is very simple as it only has to check whether the image has
 * already been exposed.
 */
ImagePtr	SxCcd::getRawImage() {
	if (state() != CcdState::exposed) {
		throw BadState("no exposure available");
	}
	thread.join();
	state(CcdState::idle);
	return image;
}

/**
 * \brief Find out whether the Ccd has a cooler
 */
bool	SxCcd::hasCooler() const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "checking for cooler");
	if (getInfo().getId() == 0) {
		return camera.hasCooler();
	}
	return false;
}

/**
 * \brief Get the thermoelectric cooler
 *
 * This method builds a SxCooler objects and wraps in a CoolerPtr smart
 * pointer.
 */
CoolerPtr	SxCcd::getCooler0() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "request for cooler");
	try {
		return camera.getCooler(ccdindex);
	} catch (std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cooler problem: %s", x.what());
		throw NotImplemented("no cooler");
	}
}

/**
 * \brief Clear all the pixels
 */
void	SxCcd::clearPixels() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "clear pixels");
	EmptyRequest    clearrequest(
		RequestBase::vendor_specific_type,
		RequestBase::device_recipient, (uint16_t)0,
		(uint8_t)SX_CMD_CLEAR_PIXELS,
		(uint16_t)CCD_EXP_FLAGS_NOWIPE_FRAME);
	camera.reserve("exposure", 1000);
	try {
		camera.controlRequest(&clearrequest);
	} catch (USBError& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "USB failed, refreshing");
		refresh();
		throw x;
	}
	camera.release("exposure");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "pixels cleared");
}

/**
 * \brief Start an Exposure on a "normal" Starlight Express camera
 *
 * \param exposure	specification of the exposure to take
 */
void	SxCcd::startExposure0(const Exposure& exposure) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start exposure %s",
		exposure.toString().c_str());

	// remember the exposure
	this->exposure = exposure;

	// we should check that the selected binning mode is in fact 
	// available
	if (!info.modes().permits(exposure.mode())) {
		debug(LOG_ERR, DEBUG_LOG, 0, "binning mode %s not supported",
			exposure.mode().toString().c_str());
		throw SxError("binning mode not supported");
	}

	// if this is an interline CCD, we should send a clear before we start
	// an exposure, maybe allways
	if ((camera.hasInterlineCcd()) && (getInfo().getId() == 0)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "extra clear for interline "
			"cameras");
		clearPixels();
	}

	// create the exposure request
	sx_read_pixels_delayed_t	rpd;
	rpd.x_offset = exposure.x();
	// here is the problem with the y-offset: since our application
	// always uses a mathematical coordinate system (just us the FITS
	// file format does), we have to flip the y offset when computing
	// the subframe
	rpd.y_offset = info.size().height()
		- (exposure.height() + exposure.y());
	rpd.width = exposure.width();
	rpd.height = exposure.height();
	rpd.x_bin = exposure.mode().x();
	rpd.y_bin = exposure.mode().y();
	rpd.delay = 1000 * exposure.exposuretime();

	// prepare the flags
	uint16_t	flags = 0;
	if (info.shutter()) {
		if (exposure.shutter() == Shutter::OPEN) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "setting shutter open");
			flags = CCD_EXP_FLAGS_SHUTTER_OPEN
				| CCD_EXP_FLAGS_SHUTTER_MANUAL;
		} else {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "setting shutter closed");
			flags = CCD_EXP_FLAGS_SHUTTER_CLOSE
				| CCD_EXP_FLAGS_SHUTTER_MANUAL;
		}
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no shutter");
	}

	// build a control request
	Request<sx_read_pixels_delayed_t>	request(
		RequestBase::vendor_specific_type,
		RequestBase::device_recipient, ccdindex,
		(uint8_t)SX_CMD_READ_PIXELS_DELAYED, flags, &rpd);
	try {
		camera.controlRequest(&request);
	} catch (USBError& x) {
		camera.release("exposure");
		std::string	msg = stringprintf("%s usb error: %s",
			name().toString().c_str(), x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw DeviceTimeout(msg);
	}

	// we are now in exposing state
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera now exposing");
	state(CcdState::exposing);
}

/**
 * \brief Retrieve an image with short pixel values.
 *
 * Starlight Express cameras always use 16 bit pixels, it is natural to
 * always produce 16 bit deep images.
 */
void	SxCcd::getImage0() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start getImage0");

	// if this exposure has flood purpose, let some other function handle
	// this request
	if (camera.hasRBIFlood() && (exposure.purpose() == Exposure::flood)) {
		doFlood(exposure);
		return;
	}

	// start the exposure
	state(CcdState::exposing);
	this->startExposure0(exposure);

	// wait a little bit of time before performing the data transfer
	double	waittime = exposure.exposuretime() - 0.1;
	if (waittime > 0) {
		Timer::sleep(waittime);
	}

	// compute the target image size, using the binning mode
	ImageSize	targetsize = exposure.size() / exposure.mode();

	// compute the size of the buffer, and create a buffer for the data
	int	size = targetsize.getPixels();
	unsigned short	*data = new unsigned short[size];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "data size to retrieve: %d",
		sizeof(unsigned short) * size);

	// when the transfer completes, one can use the data for the image
	// for the time being we construct the image, but this is mainly
	// to make sure that the data is properly cleaned up if an error
	// occurs
	Image<unsigned short>	*_image
		= new Image<unsigned short>(targetsize, data);
	_image->setOrigin(exposure.origin());
	image = ImagePtr(_image);

	// read the data from the data endpoint
	BulkTransfer	transfer(camera.getEndpoint(),
		sizeof(unsigned short) * size, (unsigned char *)data);

	// timeout depends on the actual data size we want to transfer
	int	timeout = 1100 * exposure.exposuretime() + 30000;
	transfer.setTimeout(timeout);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "data transfer timeout: %d", timeout);

	// submit the transfer
	try {
		camera.getDevicePtr()->submit(&transfer);
	} catch (USBError& x) {
		// release the camera
		camera.release("exposure");
		std::string	msg = stringprintf("%s usb error: %s",
			name().toString().c_str(), x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		state(CcdState::idle);

		// throw the exception
		throw DeviceTimeout(msg);
	}
	camera.release("exposure");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "received %d pixels", size);

	// if this is a color camera (which we can find out from the model
	// of the camera), then we should add RGB information to the image
	// but only in 1x1 binning mode
	if ((camera.isColor()) && (exposure.mode() == Binning())) {
		// add bayer info, this depends on the subframe we are
		// requesting
		_image->setMosaicType(
			(MosaicType::mosaic_type)(
			MosaicType::BAYER_RGGB \
				| ((exposure.x() % 2) << 1) \
				| (exposure.y() % 2)));
	}

	// images are upside down, since our origin is always the lower
	// left corner. Note that Hyperstar images are reversed!
	FlipOperator<unsigned short>	f;
	f(*_image);

	// if the exposure requests a limiting function, we apply it now
	if (exposure.limit() < INFINITY) {
		for (unsigned int offset = 0;
			offset < _image->size().getPixels();
			offset++) {
			unsigned short	pv = _image->pixels[offset];
			if (pv > exposure.limit()) {
				pv = exposure.limit();
			}
			_image->pixels[offset] = pv;
		}
	}

	// add the metadata
	addMetadata(*_image);

	// signal to the rest of the world that we have an image ready
	state(CcdState::exposed);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "status set to exposed");
}

/**
 * \brief Start/stop the flooding command
 */
void	SxCcd::flood(bool onoff) {
	EmptyRequest    floodrequest(
		RequestBase::vendor_specific_type,
		RequestBase::device_recipient, (uint16_t)0,
		(uint8_t)SX_CMD_FLOOD_CCD, (uint16_t)((onoff) ? 1 : 0));
	camera.reserve("exposure", 1000);
	try {
		camera.controlRequest(&floodrequest);
	} catch (USBError& x) {
		refresh();
		throw x;
	}
	camera.release("exposure");
}

/**
 * \brief Perform the RBI flood procedure
 */
void	SxCcd::doFlood(const Exposure& exposure) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "doFlood started");
	state(CcdState::exposing);
	this->exposure = exposure;

	// turn on the flood iluminator
	debug(LOG_DEBUG, DEBUG_LOG, 0, "turning RBI flood on");
	flood(true);

	// wait for exposure time
	Timer::sleep(exposure.exposuretime());

	// turn the flood illuminator off
	debug(LOG_DEBUG, DEBUG_LOG, 0, "turning RBI flood off");
	flood(false);

	// now clear the pixels
	clearPixels();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "pixels cleared");

	// create an empty image
	image = ImagePtr(new Image<unsigned short>(ImageSize(1, 1)));
	state(CcdState::exposed);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "doFlood complete");
}

/**
 * \brief get the user friendly name of the camera
 */
std::string	SxCcd::userFriendlyName() const {
	return camera.userFriendlyName();
}

/**
 * \brief Refresh the connection
 */
void	SxCcd::refresh() {
	camera.refresh();
}

} // namespace sx
} // namespace camera
} // namespace astro
