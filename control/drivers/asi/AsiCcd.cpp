/*
 * AsiCcd.cpp -- implementation of asi ccd
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rappreswil
 */

#include <AsiCcd.h>
#include <ASICamera2.h>
#include <AsiCooler.h>
#include <AsiStream.h>
#include <AstroExceptions.h>
#include <includes.h>

namespace astro {
namespace camera {
namespace asi {

/**
 * \brief Construct a new CCD object
 *
 * \param info		Information about the CCD
 * \param camera	Camera to which this CCD belongs
 */
AsiCcd::AsiCcd(const CcdInfo& info, AsiCamera& camera)
	: Ccd(info), _camera(camera) {
	stream = NULL;
	_thread = NULL;
	_exposure_done = true;
}

/**
 * \brief Destroy the CCD object
 */
AsiCcd::~AsiCcd() {
	if (stream) {
		delete stream;
	}
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
void	AsiCcd::setExposure(const Exposure& e) {
	exposure = e;
	if (exposure.size() == ImageSize()) {
		exposure.frame(info.size());
	} else {
		// make sure window sizes are divisible by 8/2
		int	w = exposure.width();
		w = ((w + 7) >> 3) << 3;
		int	h = exposure.height();
		h = ((h + 1) >> 1) << 1;
		exposure.frame(ImageRectangle(exposure.origin(),
			ImageSize(w, h)));
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set exposure %s -> %s",
		e.toString().c_str(), exposure.toString().c_str());
	ImageSize	sensorsize = info.size() / exposure.mode();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "sensor size: %s",
		sensorsize.toString().c_str());

	// set ROI
	debug(LOG_DEBUG, DEBUG_LOG, 0, "origin: %s, mode: %s",
		exposure.frame().origin().toString().c_str(),
		exposure.mode().toString().c_str());
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

	// show the density
	debug(LOG_DEBUG, DEBUG_LOG, 0, "info: %s", info.toString().c_str());

	// set start position
	int	y = info.size().height() - (origin.y() + roi.size.height());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set start: %s",
		origin.toString().c_str());
	_camera.setStartPos(ImagePoint(origin.x(), y));
	//_camera.setStartPos(origin);

	// set the exposure time
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set exposure time: %.3f",
		exposure.exposuretime());
	AsiControlValue	value;
	value.type = AsiExposure;
	value.value = 1000000 * exposure.exposuretime();
	value.isauto = false;
	_camera.setControlValue(value);

	// XXX set the gain
	exposure.gain(_camera.getControlValue(AsiGain).value);

	// that's it
	debug(LOG_DEBUG, DEBUG_LOG, 0, "exposure settings complete");
}

static void     start_main(AsiCcd *asiccd) {
        asiccd->run();
}

/**
 * \brief Start a single exposure
 *
 * \param exposure	exposure structure to use for the single exposure
 */
void	AsiCcd::startExposure(const Exposure& exposure) {
	// the lock ensures that there can only ever by one thread inside
	// this method. If the thread has already been started the exposure
	// status is no longer compatible with starting an exposure and
	// the second startExposure will fail.
	std::lock_guard<std::recursive_mutex>	lock(_mutex);
	if (streaming()) {
		std::string	msg("camera is currently streaming");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw BadState(msg);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s start exposure %s",
		name().toString().c_str(), exposure.toString().c_str());

	// call the Ccds startExposure method, this ensures we are presently
	// in the correct state,
	Ccd::startExposure(exposure);
	try {
		setExposure(exposure);

		// start the exposure
		_camera.startExposure(exposure.shutter() == Shutter::OPEN);
	} catch (...) {
	}
	//exposureStatus();

	_exposure_done = false;
	_thread = new std::thread(start_main, this);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "exposure started");
}

/**
 * \brief Cancel an image that is already in progress
 */
void	AsiCcd::cancelExposure() {
	_camera.stopExposure();
	state(CcdState::cancelling);
}

/**
 * \brief Main function of the asi exposure thread
 */
void	AsiCcd::run() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start thread");
	double	starttime = Timer::gettime();
	double	step = 10;
	do {
		double	remaining = exposure.exposuretime()
					- (Timer::gettime() - starttime);
		if ((remaining > 0) && (remaining < 10)) {
			step = remaining - 0.1;
		}
		if (step < 0) {
			step = 0.1;
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "state %s, sleep for %.3fsec",
			CcdState::state2string(exposureStatus()).c_str(),
			step);
		Timer::sleep(step);
	} while (CcdState::exposing == exposureStatus());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "no longer exposing");
	std::lock_guard<std::recursive_mutex>	lock(_mutex);
	_exposure_done = true;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "exposing finished");
}

/**
 * \brief Query the exposure status
 */
CcdState::State	AsiCcd::exposureStatus() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "exposureStatus()");
	std::lock_guard<std::recursive_mutex>	lock(_mutex);
	if ((_thread) && (_exposure_done)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "thread cleanup");
		_thread->join();
		delete _thread;
		_thread = NULL;
	}
	ASI_EXPOSURE_STATUS	status = _camera.getExpStatus();
	switch (status) {
	case ASI_EXP_IDLE:
		if (Asi_Debug_State) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "%s is IDLE/idle",
				name().toString().c_str());
		}
		state(CcdState::idle);
		break;
	case ASI_EXP_WORKING:
		if (Asi_Debug_State) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "%s is WORKING/exposing",
				name().toString().c_str());
		}
		if (state() != CcdState::exposing) {
			state(CcdState::exposing);
		}
		break;
	case ASI_EXP_SUCCESS:
		if (Asi_Debug_State) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "%s is SUCCESS/exposed",
				name().toString().c_str());
		}
		if (state() != CcdState::exposed) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "changing state to exposed");
			state(CcdState::exposed);
		}
		break;
	case ASI_EXP_FAILED:
		if (Asi_Debug_State) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "%s is FAILED/idle",
				name().toString().c_str());
		}
		if (CcdState::idle != state()) {
			state(CcdState::idle);
		}
		break;
	default:
		{
		std::string	msg = stringprintf("unknown ASI status: %d",
			status);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
		}
	}
	return state();
}

/**
 * \brief get an Image from the camera
 */
astro::image::ImagePtr	AsiCcd::getRawImage() {
	// make sure we are in the right mode
	if (_camera.asi_mode() == AsiCamera::mode_idle) {
		std::string	msg("camera is idle, cannot get raw");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// get the pixel size
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get a raw image");
	ASI_IMG_TYPE	imgtype = string2imgtype(imgtypename());
	int	pixelsize = 1;
	switch (imgtype) {
	case ASI_IMG_RGB24:
		pixelsize = 3;
		break;
	case ASI_IMG_RAW16:
		pixelsize = 2;
		break;
	default:
		break;
	}

	// the the image size
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get raw image from exposure %s",
		exposure.toString().c_str());
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

	// geth the image data
	debug(LOG_DEBUG, DEBUG_LOG, 0, "buffer at %p", buffer);
	switch (_camera.asi_mode()) {
	case AsiCamera::mode_exposure:
		_camera.getDataAfterExp(buffer, buffersize);
		break;
	case AsiCamera::mode_stream:
		_camera.getVideoData(buffer, buffersize,
			1000 * exposure.exposuretime() + 1000);
		break;
	default:
		break;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got the image data");

	// convert this into an Image of the appropriate type
	int	h = size.height();
	ImagePtr	result;
	switch (imgtype) {
	case ASI_IMG_RAW8: // convert 8bit mono image to Image<unsigned char>
	case ASI_IMG_Y8: // convert 8bit mono image to Image<unsigned char>
		debug(LOG_DEBUG, DEBUG_LOG, 0, "get Y8/RAW8 image");
		{
		Image<unsigned char>	*image = new Image<unsigned char>(size);
		for (int x = 0; x < size.width(); x++) {
			for (int y = 0; y < size.height(); y++) {
				image->pixel(x, h - 1 - y)
					= buffer[x + size.width() * y];
			}
		}
		// if this is a color camera, add the mosaic information,
		// at least for the raw camera
		if ((_camera.isColor()) && (imgtype == ASI_IMG_RAW8)) {
			image->setMosaicType(
				MosaicType::shift(MosaicType::BAYER_RGGB,
					origin));
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
		// if this is a color camera, add the mosaic information
		if (_camera.isColor()) {
			image->setMosaicType(_camera.mosaic().shifted(origin));
		}
		result = ImagePtr(image);
		}
		break;
	default: {
		std::string	msg = stringprintf("%s: unknown type %d",
			name().toString().c_str(), imgtype);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
		}
	}
	result->setOrigin(origin);
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
void    AsiCcd::streamExposure(const Exposure& e) {
	setExposure(e);
	ImageStream::streamExposure(e);
}

/**
 * \brief Start streaming
 */
void    AsiCcd::startStream(const Exposure& exposure) {
	if (NULL != stream) {
		throw std::runtime_error("stream already running");
	}
	streamExposure(exposure);
	_camera.startVideoCapture();
	stream = new AsiStream(this);
}

/**
 * \brief Stop streaming
 */
void    AsiCcd::stopStream() {
	stream->stop();
	delete stream;
	_camera.stopVideoCapture();
	stream = NULL;
}

bool	AsiCcd::streaming() {
	return (NULL != stream);
}

/**
 * \brief Get the gain
 */
float	AsiCcd::getGain() {
	return (float)_camera.getControlValue(AsiGain).value;
}

/**
 * \brief Retrieve interval of valid gain values
 */
std::pair<float, float>	AsiCcd::gainInterval() {
	return std::make_pair(  (float)_camera.controlMin(AsiGain),
				(float)_camera.controlMax(AsiGain));
}

/**
 * \brief Find out whether the camera knows the temperature
 */
bool	AsiCcd::hasTemperature() {
	try {
		_camera.controlIndex("Temperature");
		return true;
	} catch (const std::exception& x) {
	}
	return false;
}

/**
 * \brief Get the temperature
 */
float	AsiCcd::getTemperature() {
	return Temperature::zero + _camera.getControlValue(AsiTemperature).value / 10.;
}

} // namespace asi
} // namespace camera
} // namespace astro
