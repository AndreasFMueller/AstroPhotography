/*
 * AsiCcd.cpp -- implementation of asi ccd
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rappreswil
 */

#include <AsiCcd.h>
#include <ASICamera2.h>
#include <AsiCooler.h>
#include <AsiStream.h>
#include <errno.h>
#include <string.h>

namespace astro {
namespace camera {
namespace asi {

/**
 * \brief Construct a new CCD object
 */
AsiCcd::AsiCcd(const CcdInfo& info, AsiCamera& camera)
	: Ccd(info), _camera(camera) {
}

/**
 * \brief Destroy the CCD object
 */
AsiCcd::~AsiCcd() {
}

/**
 * \brief Access the unit name, which is also the imgtype
 */
std::string	AsiCcd::imgtypename() {
	return name().unitname();
}

/**
 * \brief convert image type to a string representation
 */
std::string	AsiCcd::imgtype2string(int imgtype) {
	switch (imgtype) {
	case ASI_IMG_RAW8:
		return std::string("raw8");
	case ASI_IMG_RGB24:
		return std::string("rgb24");
	case ASI_IMG_RAW16:
		return std::string("raw16");
	case ASI_IMG_Y8:
		return std::string("y8");
	}
	throw std::runtime_error("unknown image type");
}

/**
 * \brief convert name of image typ to ASI_IMG_* code
 */
static ASI_IMG_TYPE	string2imgtype(const std::string& imgname) {
	if (imgname == "raw8") {
		return ASI_IMG_RAW8;
	}
	if (imgname == "rgb24") {
		return ASI_IMG_RGB24;
	}
	if (imgname == "raw16") {
		return ASI_IMG_RAW16;
	}
	if (imgname == "y8") {
		return ASI_IMG_Y8;
	}
	throw std::runtime_error("unknown image name");
}

/**
 * \brief Set the exposure data
 */
void	AsiCcd::setExposure(const Exposure& exposure) {
#if 0
	int	rc;
	// set binning mode
	int	bin = exposure.mode().x();
#endif
	ImageSize	sensorsize = info.size() / exposure.mode();

	// set ROI
	ImagePoint	origin = exposure.frame().origin() / exposure.mode();
	ImageSize	size = exposure.frame().size() / exposure.mode();
	ImageRectangle	frame(origin, size);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "set ROI %s",
		frame.toString().c_str());
	AsiCamera::roi_t	roi;
	roi.size = frame.size();
	roi.mode = exposure.mode();
	roi.img_type = string2imgtype(imgtypename());
	_camera.setROIFormat(roi);
#if 0
	if (ASI_SUCCESS != (rc = ASISetROIFormat(_camera.id(),
		frame.size().width(), frame.size().height(),
		bin, string2imgtype(imgtypename())))) {
		std::string	msg = stringprintf("%s cannot set "
			"ROI: %s",
			_camera.name().toString().c_str(),
			AsiCamera::error(rc).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
#endif
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set start: %s",
		origin.toString().c_str());
	_camera.setStartPos(origin);
#if 0
	if (ASI_SUCCESS != (rc = ASISetStartPos(_camera.id(),
		origin.x(), origin.y()))) {
		std::string	msg = stringprintf("%s cannot set "
			"start position %s",
			_camera.name().toString().c_str(),
			AsiCamera::error(rc).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
#endif

	// set the exposure time
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set exposure time");
	AsiControlValue	value;
	value.type = AsiExposure;
	value.value = 1000000 * exposure.exposuretime();
	value.isauto = false;
	_camera.setControlValue(value);

	// XXX set the gain
}

/**
 * \brief Start a single exposure
 */
void	AsiCcd::startExposure(const Exposure& exposure) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s start exposure %s",
		name().toString().c_str(), exposure.toString().c_str());
#if 0
	int	rc;
#endif
	Ccd::startExposure(exposure);
	try {
		setExposure(exposure);

		// start the exposure
#if 0
		ASI_BOOL	isdark = (exposure.shutter() == Shutter::OPEN)
						? ASI_FALSE : ASI_TRUE;
#endif
		_camera.startExposure(exposure.shutter() == Shutter::OPEN);
#if 0
		if (ASI_SUCCESS != (rc = ASIStartExposure(_camera.id(),
			isdark))) {
			std::string	msg = stringprintf("%s cannot start "
				"exposure: %s",
				_camera.name().toString().c_str(),
				AsiCamera::error(rc).c_str());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
#endif
	} catch (const std::exception& x) {
		Ccd::cancelExposure();
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "exposure started");
}

/**
 * \brief Cancel an image that is already in progress
 */
void	AsiCcd::cancelExposure() {
	_camera.stopExposure();
#if 0
	ASIStopExposure(_camera.id());
#endif
}

/**
 * \brief Query the exposure status
 */
CcdState::State	AsiCcd::exposureStatus() {
	ASI_EXPOSURE_STATUS	status = _camera.getExpStatus();
#if 0
	if (ASI_SUCCESS != ASIGetExpStatus(_camera.id(), &status)) {
		std::string	msg = stringprintf("cannot get exp status @ %d",
			_camera.id());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
#endif
	switch (status) {
	case ASI_EXP_IDLE:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s is IDLE/idle",
			name().toString().c_str());
		state(CcdState::idle);
		return CcdState::idle;
	case ASI_EXP_WORKING:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s is WORKING/exposing",
			name().toString().c_str());
		state(CcdState::exposing);
		return CcdState::exposing;
	case ASI_EXP_SUCCESS:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s is SUCCESS/exposed",
			name().toString().c_str());
		state(CcdState::exposed);
		return CcdState::exposed;
	case ASI_EXP_FAILED:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s is FAILED/exposed",
			name().toString().c_str());
		state(CcdState::exposed);
		return CcdState::exposed;
	}
	std::string	msg = stringprintf("unknown ASI status: %d", status);
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

/**
 * \brief get an Image from the camera
 */
astro::image::ImagePtr	AsiCcd::getRawImage() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get a raw image");
#if 0
	int	rc;
#endif
	ASI_IMG_TYPE	imgtype = string2imgtype(imgtypename());
	int	pixelsize = 1;
	switch (imgtype) {
	case ASI_IMG_RGB24:
		pixelsize = 3;
		break;
	case ASI_IMG_RAW16:
	case ASI_IMG_Y8:
		pixelsize = 2;
		break;
	default:
		break;
	}

	ImagePoint	origin = exposure.frame().origin() / exposure.mode();
	ImageSize	size = exposure.frame().size() / exposure.mode();
	ImageRectangle	frame(origin, size);
	long	buffersize = size.getPixels() * pixelsize;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "pixel size: %d, buffer size: %ld",
		pixelsize, buffersize);
	unsigned char	*buffer = (unsigned char *)malloc(buffersize);
	if (NULL == buffer) {
		std::string	msg = stringprintf("cannot allocate buffer: %s",
			strerror(errno));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "buffer at %p", buffer);
	_camera.getDataAfterExp(buffer, buffersize);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got the image data");

	// convert this into an Image of the appropriate type
	int	h = size.height();
	ImagePtr	result;
	switch (imgtype) {
	case ASI_IMG_RAW8: // convert 8bit mono image to Image<unsigned char>
		debug(LOG_DEBUG, DEBUG_LOG, 0, "get RAW8 image");
		{
		Image<unsigned char>	*image = new Image<unsigned char>(size);
		for (int x = 0; x < size.width(); x++) {
			for (int y = 0; y < size.height(); y++) {
				image->pixel(x, h - 1 - y)
					= buffer[x + size.width() * y];
			}
		}
		result = ImagePtr(image);
		}
		break;
	case ASI_IMG_RGB24: // convert 8bit color image to Image<RGB<unsigned char> >
		debug(LOG_DEBUG, DEBUG_LOG, 0, "get RGB24 image");
		{
		Image<RGB<unsigned char> >	*image
			= new Image<RGB<unsigned char> >(size);
		for (int x = 0; x < size.width(); x++) {
			for (int y = 0; y < size.height(); y++) {
				long	offset = (x + size.width() * y) * 3;
				image->pixel(x, h - 1 - y) = RGB<unsigned char>(buffer[offset + 2], buffer[offset + 1], buffer[offset + 0]);
			}
		}
		result = ImagePtr(image);
		}
		break;
	case ASI_IMG_RAW16: // convert 16bit mono image to Image<unsigned short>
		debug(LOG_DEBUG, DEBUG_LOG, 0, "get RAW16 image");
		{
		Image<unsigned short>	*image
			= new Image<unsigned short>(size);
		unsigned short	*sb = (unsigned short *)buffer;
		for (int x = 0; x < size.width(); x++) {
			for (int y = 0; y < size.height(); y++) {
				image->pixel(x, h - 1 - y)
					= sb[x + size.width() * y];
			}
		}
		result = ImagePtr(image);
		}
		break;
	case ASI_IMG_Y8: // convert 8bit YUYV image to Image<YUYV<unsigned char> >
		debug(LOG_ERR, DEBUG_LOG, 0, "Y8 format not implemented");
		throw std::runtime_error("Y8 format not implemented");
		break;
	default: {
		std::string	msg = stringprintf("%s: unknown type %d",
			name().toString().c_str(), imgtype);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
		}
	}
	free(buffer);
	return result;
}

/**
 * \brief Get a cooler for this CCD
 */
CoolerPtr	AsiCcd::getCooler0() {
	return CoolerPtr(new AsiCooler(_camera, *this));
}

/**
 * \brief Set the exposure parameters for the stream
 */
void    AsiCcd::streamExposure(const Exposure& exposure) {
	ImageStream::streamExposure(exposure);
}

/**
 * \brief Start streaming
 */
void    AsiCcd::startStream(const Exposure& /* exposure */) {
	if (NULL != stream) {
		throw std::runtime_error("stream already running");
	}
	streamExposure(exposure);
	stream = new AsiStream(this);
}

/**
 * \brief Stop streaming
 */
void    AsiCcd::stopStream() {
	delete stream;
}

} // namespace asi
} // namespace camera
} // namespace astro
