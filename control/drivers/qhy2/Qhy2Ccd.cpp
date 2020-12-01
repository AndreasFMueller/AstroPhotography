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
#include <qhyccd.h>

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

double	Qhy2Ccd::getExposuretime(float exposuretime) {
	double	min, max, step;
	if (QHYCCD_SUCCESS == GetQHYCCDParamMinMaxStep(camera.handle(),
			CONTROL_EXPOSURE, &min, &max, &step)) {
		if (exposuretime < min / 1000000.) {
			return min / 1000000.;
		}
		if (exposuretime > max / 1000000.) {
			return max / 1000000.;
		}
		return (min + round((1000000 * exposuretime - min) / step) * step);
	} else {
		if (exposuretime < info.minexposuretime()) {
			return info.minexposuretime();
		}
		if (exposuretime > info.maxexposuretime()) {
			return info.maxexposuretime();
		}
		return exposuretime;
	}
}

/**
 * \brief class specific image retrieval from the QHY camera
 */
void	Qhy2Ccd::getImage0() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "starting getImage0()");
	state(CcdState::exposing);

	// set single frame mode
	int	rc = SetQHYCCDStreamMode(camera.handle(), 0);
	if (rc != QHYCCD_SUCCESS) {
		std::string	msg = stringprintf("cannot set stream mode "
			"in %s", camera.qhyname().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		state(CcdState::idle);
		throw Qhy2Error(msg, rc);
	}

	// find and set the correct exposure time
	double	exposuretime = getExposuretime(exposure.exposuretime());
	rc = SetQHYCCDParam(camera.handle(), CONTROL_EXPOSURE,
			1000000. * exposuretime);
	if (rc != QHYCCD_SUCCESS) {
		std::string	msg = stringprintf("cannot set exposure time "
			"in %s", camera.qhyname().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		state(CcdState::idle);
		throw Qhy2Error(msg, rc);
	}
	exposure.exposuretime(exposuretime);

	// apply the gain setting, if available
	if (IsQHYCCDControlAvailable(camera.handle(), CONTROL_GAIN)) {
		rc = SetQHYCCDParam(camera.handle(), CONTROL_GAIN,
			exposure.gain());
		if (rc != QHYCCD_SUCCESS) {
			std::string	msg = stringprintf("cannot set gain "
				"in %s", camera.qhyname().c_str());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			state(CcdState::idle);
			throw Qhy2Error(msg, rc);
		}
	}

	// XXX apply the offset setting, if available

	// find the region of interest and set it, if possible. Also
	// remember whether we will have to extract the region of interest
	// after reading the image from the camera
	rc = SetQHYCCDResolution(camera.handle(), exposure.x(), exposure.y(),
		exposure.width(), exposure.height());
	if (rc != QHYCCD_SUCCESS) {
		std::string	msg = stringprintf("cannot set image size %s "
			"in %s", exposure.frame().toString().c_str(),
			camera.qhyname().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		state(CcdState::idle);
		throw Qhy2Error(msg, rc);
	}

	// set the binning mode
	rc = SetQHYCCDBinMode(camera.handle(), exposure.mode().x(),
		exposure.mode().y());
	if (rc != QHYCCD_SUCCESS) {
		std::string	msg = stringprintf("cannot set binning mode "
			"in %s", camera.qhyname().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		state(CcdState::idle);
		throw Qhy2Error(msg, rc);
	}

	// find and set the transfer bit mode
	if (IsQHYCCDControlAvailable(camera.handle(), CONTROL_TRANSFERBIT)) {
		int	bits = std::stoi(name().unitname());
		rc = SetQHYCCDBitsMode(camera.handle(), bits);
		if (rc != QHYCCD_SUCCESS) {
			std::string	msg = stringprintf("cannot set bit "
				"depth in %s", camera.qhyname().c_str());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			state(CcdState::idle);
			throw Qhy2Error(msg, rc);
		}
	}

	// XXX handle the shutter

	// start the actual exposure
	rc = ExpQHYCCDSingleFrame(camera.handle());
	if (rc == 0) {
		std::string	msg = stringprintf("cannot start exposure "
			"in %s", camera.qhyname().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		state(CcdState::idle);
		throw Qhy2Error(msg, rc);
	}

	// get the memory size needed for the buffer
	uint32_t	length = GetQHYCCDMemLength(camera.handle());
	if (length == 0) {
		std::string	msg = stringprintf("cannot get length for '%s'",
			camera.qhyname().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		state(CcdState::idle);
		throw Qhy2Error(msg, rc);
	}
	unsigned char	*imagedata = new unsigned char[length];

	// read the image from the camera
	unsigned int	imagewidth, imageheight, bpp, channels;
	rc = GetQHYCCDSingleFrame(camera.handle(), &imagewidth, &imageheight,
		&bpp, &channels, imagedata);
	if (rc != QHYCCD_SUCCESS) {
		std::string	msg = stringprintf("cannot get image data for "
			"'%s'", camera.qhyname().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		state(CcdState::idle);
		throw Qhy2Error(msg, rc);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %ux%u image bpp=%u channels=%u",
		imagewidth, imageheight, bpp, channels);

	// throw away an old image
	image.reset();

	// convert the image data to an image
	ImageSize	resultsize(imagewidth, imageheight);
	switch (bpp) {
	case 8:	{
		Image<unsigned char>	*imagecontent
			= new Image<unsigned char>(resultsize);
		for (unsigned int x = 0; x < imagewidth; x++) {
			for (unsigned int y = 0; y < imageheight; y++) {
				imagecontent->pixel(x, y)
					= imagedata[y * imagewidth + x];
			}
		}
		image = ImagePtr(imagecontent);
		}
		break;
	case 16:{
		Image<unsigned short>	*imagecontent
			= new Image<unsigned short>(resultsize);
		unsigned short	*shortimagedata = (unsigned short *)imagedata;
		for (unsigned int x = 0; x < imagewidth; x++) {
			for (unsigned int y = 0; y < imageheight; y++) {
				imagecontent->pixel(x, y)
					= shortimagedata[y * imagewidth + x];
			}
		}
		image = ImagePtr(imagecontent);
		}
		break;
	}

	// what to do if we have no image
	if (!image) {
		std::string	msg = stringprintf("no image found");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		state(CcdState::idle);
		throw Qhy2Error(msg, -1);
	}
	
	// add the color mosaic code if present
	switch (IsQHYCCDControlAvailable(camera.handle(), CAM_COLOR)) {
	case BAYER_GB:
		image->setMosaicType(MosaicType(MosaicType::BAYER_GBRG));
		break;
	case BAYER_GR:
		image->setMosaicType(MosaicType(MosaicType::BAYER_GRBG));
		break;
	case BAYER_BG:
		image->setMosaicType(MosaicType(MosaicType::BAYER_BGGR));
		break;
	case BAYER_RG:
		image->setMosaicType(MosaicType(MosaicType::BAYER_RGGB));
		break;
	default:
		image->setMosaicType(MosaicType(MosaicType::NONE));
		break;
	}

	// terminate the process on the camera size
	rc = CancelQHYCCDExposingAndReadout(camera.handle());
	if (rc != QHYCCD_SUCCESS) {
		std::string	msg = stringprintf("cannot get image data for "
			"'%s'", camera.qhyname().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw Qhy2Error(msg, rc);
	}

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
