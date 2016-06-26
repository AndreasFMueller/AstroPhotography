/*
 * AsiCcd.cpp -- implementation of asi ccd
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rappreswil
 */

#include <AsiCcd.h>
#include <ASICamera2.h>
#include <AsiCooler.h>

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
 * \brief Start a single exposure
 */
void	AsiCcd::startExposure(const Exposure& exposure) {
	int	rc;
	Ccd::startExposure(exposure);
	// set binning mode
	int	bin = exposure.mode().x();
	ImageSize	sensorsize = info.size() / exposure.mode();

	// set ROI
	ImagePoint	origin = exposure.frame().origin() / exposure.mode();
	ImageSize	size = exposure.frame().size() / exposure.mode();
	ImageRectangle	frame(origin, size);
	try {
		if (ASI_SUCCESS != (rc = ASISetROIFormat(_camera.id(),
			frame.size().width(), frame.size().height(),
			bin, string2imgtype(imgtypename())))) {
		}
		if (ASI_SUCCESS != (rc = ASISetStartPos(_camera.id(),
			origin.x(), origin.y()))) {
		}
	
		// set the exposure time
		AsiControlValue	value;
		value.type = AsiExposure;
		value.value = 1000 * exposure.exposuretime();
		value.isauto = false;
		_camera.setControlValue(value);

		// XXX set the gain

		// start the exposure
		ASI_BOOL	isdark = (exposure.shutter() == Shutter::OPEN)
						? ASI_FALSE : ASI_TRUE;
		ASIStartExposure(_camera.id(), isdark);
	} catch (const std::exception& x) {
		Ccd::cancelExposure();
	}
}

/**
 * \brief Cancel an image that is already in progress
 */
void	AsiCcd::cancelExposure() {
	ASIStopExposure(_camera.id());
}

/**
 * \brief Query the exposure status
 */
CcdState::State	AsiCcd::exposureStatus() {
	ASI_EXPOSURE_STATUS	status;
	if (ASI_SUCCESS != ASIGetExpStatus(_camera.id(), &status)) {
		std::string	msg = stringprintf("cannot get exp status @ %d",
			_camera.id());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	switch (status) {
	case ASI_EXP_IDLE:
		return CcdState::idle;
	case ASI_EXP_WORKING:
		return CcdState::exposing;
	case ASI_EXP_SUCCESS:
		return CcdState::exposed;
	case ASI_EXP_FAILED:
		return CcdState::exposed;
	}
}

/**
 * \brief get an Image from the camera
 */
astro::image::ImagePtr	AsiCcd::getRawImage() {
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
	unsigned char	*buffer = (unsigned char *)alloca(buffersize);
	ASIGetDataAfterExp(_camera.id(), buffer, buffersize);

	// convert this into an Image of the appropriate type
	switch (imgtype) {
	case ASI_IMG_RAW8: // convert 8bit mono image to Image<unsigned char>
		{
		Image<unsigned char>	*image = new Image<unsigned char>(size);
		for (int x = 0; x < size.width(); x++) {
			for (int y = 0; y < size.height(); y++) {
				image->pixel(x, y)
					= buffer[x + size.width() * y];
			}
		}
		return ImagePtr(image);
		}
	case ASI_IMG_RGB24: // convert 8bit color image to Image<RGB<unsigned char> >
		{
		Image<RGB<unsigned char> >	*image
			= new Image<RGB<unsigned char> >(size);
		for (int x = 0; x < size.width(); x++) {
			for (int y = 0; y < size.height(); y++) {
				long	offset = (x + size.width() * y) * 3;
				image->pixel(x, y) = RGB<unsigned char>(buffer[offset], buffer[offset + 1], buffer[offset + 2]);
			}
		}
		return ImagePtr(image);
		}
	case ASI_IMG_RAW16: // convert 16bit mono image to Image<unsigned short>
		{
		Image<unsigned short>	*image
			= new Image<unsigned short>(size);
		unsigned short	*sb = (unsigned short *)buffer;
		for (int x = 0; x < size.width(); x++) {
			for (int y = 0; y < size.height(); y++) {
				image->pixel(x, y) = sb[x + size.width() * y];
			}
		}
		return ImagePtr(image);
		}
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
	
	return astro::image::ImagePtr(NULL);
}

CoolerPtr	AsiCcd::getCooler0() {
	return CoolerPtr(new AsiCooler(_camera, *this));
}

} // namespace asi
} // namespace camera
} // namespace astro
