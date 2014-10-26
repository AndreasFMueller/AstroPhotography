/*
 * SxCcd.cpp -- Starlight express CCD implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SxCcd.h>
#include <AstroCamera.h>
#include <AstroImage.h>
#include <AstroFilter.h>
#include <AstroOperators.h>
#include <AstroExceptions.h>
#include <sx.h>
#include <AstroDebug.h>
#include <SxUtils.h>

using namespace astro::camera;
using namespace astro::image::filter;
using namespace astro::image::operators;

namespace astro {
namespace camera {
namespace sx {

/**
 * \brief Construct an SxCcd
 */
SxCcd::SxCcd(const CcdInfo& info, SxCamera& _camera, int _ccdindex)
	: Ccd(info), camera(_camera), ccdindex(_ccdindex) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating CCD %d", ccdindex);
}

/**
 * \brief Destroy an SxCcd
 */
SxCcd::~SxCcd() {
}

/**
 * \brief Start Routine of the exposure thread
 */
void	start_routine(SxCcd *ccd) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start exposure thread");
	ccd->getImage0();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "end exposure thread");
}

/**
 * \brief Start the exposure
 *
 * This method calls the "real" startExposure0 method and launches a thread
 * that performs the getImage0 method which will retrieve the image. 
 */
void	SxCcd::startExposure(const Exposure& exposure) {
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
	if (state != Exposure::exposed) {
		throw BadState("no exposure available");
	}
	thread.join();
	state = Exposure::idle;
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
 * \brief Start an Exposure on a "normal" Starlight Express camera
 *
 * \param exposure	specification of the exposure to take
 */
void	SxCcd::startExposure0(const Exposure& exposure) {
	// remember the exposure
	this->exposure = exposure;

	// we should check that the selected binning mode is in fact 
	// available
	if (!info.modes().permits(exposure.mode)) {
		debug(LOG_ERR, DEBUG_LOG, 0, "binning mode %s not supported",
			exposure.mode.toString().c_str());
		throw SxError("binning mode not supported");
	}

	// if this is an interline CCD, we should send a clear before we start
	// an exposure, maybe allways
	if ((camera.hasInterlineCcd()) && (getInfo().getId() == 0)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "extra clear for interline "
			"cameras");
		EmptyRequest    resetrequest(
			RequestBase::vendor_specific_type,
			RequestBase::device_recipient, (uint16_t)0,
			(uint8_t)SX_CMD_CLEAR_PIXELS, (uint16_t)0);
		camera.controlRequest(&resetrequest);
	}

	// create the exposure request
	sx_read_pixels_delayed_t	rpd;
	rpd.x_offset = exposure.frame.origin().x();
	// here is the problem with the y-offset: since our application
	// always uses a mathematical coordinate system (just us the FITS
	// file format does), we have to flip the y offset when computing
	// the subframe
	rpd.y_offset = info.size().height()
		- (exposure.frame.size().height() + exposure.frame.origin().y());
	rpd.width = exposure.frame.size().width();
	rpd.height = exposure.frame.size().height();
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera now exposing");
	state = Exposure::exposing;
}

/**
 * \brief Retrieve an image with short pixel values.
 *
 * Starlight Express cameras always use 16 bit pixels, it is natural to
 * always produce 16 bit deep images.
 */
void	SxCcd::getImage0() {
	// start the exposure
	state = Exposure::exposing;
	this->startExposure0(exposure);

	// compute the target image size, using the binning mode
	ImageSize	targetsize = exposure.frame.size() / exposure.mode;

	// compute the size of the buffer, and create a buffer for the data
	int	size = targetsize.getPixels();
	unsigned short	*data = new unsigned short[size];

	// read the data from the data endpoint
	BulkTransfer	transfer(camera.getEndpoint(),
		sizeof(unsigned short) * size, (unsigned char *)data);

	// timeout depends on the actual data size we want to transfer
	int	timeout = 1100 * exposure.exposuretime + 30000;
	transfer.setTimeout(timeout);

	// submit the transfer
	camera.getDevicePtr()->submit(&transfer);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "received %d pixels", size);

	// when the transfer completes, one can use the data for the image
	Image<unsigned short>	*_image
		= new Image<unsigned short>(targetsize, data);
	_image->setOrigin(exposure.frame.origin());

	// if this is a color camera (which we can find out from the model
	// of the camera), then we should add RGB information to the image
	// but only in 1x1 binning mode
	if ((camera.isColor()) && (exposure.mode == Binning())) {
		// add bayer info, this depends on the subframe we are
		// requesting
		_image->setMosaicType(
			(MosaicType::mosaic_type)(
			MosaicType::BAYER_RGGB \
				| ((exposure.frame.origin().x() % 2) << 1) \
				| (exposure.frame.origin().y() % 2)));
	}

	// images are upside down, since our origin is always the lower
	// left corner. Note that Hyperstar images are reversed!
	FlipOperator<unsigned short>	f;
	f(*_image);

	// if the exposure requests a limiting function, we apply it now
	if (exposure.limit < INFINITY) {
		for (unsigned int offset = 0; offset < _image->size().getPixels();
			offset++) {
			unsigned short	pv = _image->pixels[offset];
			if (pv > exposure.limit) {
				pv = exposure.limit;
			}
			_image->pixels[offset] = pv;
		}
	}

	// add the metadata
	addMetadata(*_image);

	image = ImagePtr(_image);
	state = Exposure::exposed;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "status set to exposed");
}

} // namespace sx
} // namespace camera
} // namespace astro
