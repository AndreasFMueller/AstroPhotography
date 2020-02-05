/*
 * AtikCamera.cpp -- ATIK camera implementation
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AtikCamera.h>
#include <AtikUtils.h>
#include <AtikCcd.h>
#include <AtikCooler.h>
#include <AtikGuideport.h>
#include <atikccdusb.h>
#include <includes.h>

namespace astro {
namespace camera {
namespace atik {

/**
 * \brief Constructor for an ATIK camera
 */
AtikCamera::AtikCamera(::AtikCamera *camera)
	: Camera(cameraname(camera)), _camera(camera) {
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating ATIK camera %s",
		name().toString().c_str());
	// get the capabilities
	const char	*atikname;
	_camera->getCapabilities(&atikname, &_type, &_capa);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "capabilities of %s retrieved",
		atikname);
	_atikname = std::string(atikname);

	// for an Atik 383L+ Color we have to fudge the color capability,
	// as the library gives incorrect information
	if (std::string(atikname) == std::string("Atik 383L+")) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "faking color camera for %s",
			atikname);
		_capa.colour = COLOUR_RGGB;
	}
	// similarly for the Atik GP/M, which is mono
	if (std::string(atikname) == std::string("Atik GP/M")) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "faking mono camera for %s",
			atikname);
		_capa.colour = COLOUR_NONE;
	}

	// serial number
	_serial = _camera->getSerialNumber();

	// create CCD info for each CCD
	ImageSize	size(_capa.pixelCountX, _capa.pixelCountY);
	CcdInfo	info(ccdname(_camera, "Imaging"), size, 0);
	for (unsigned int binx = 1; binx <= _capa.maxBinX; binx++) {
		for (unsigned int biny = 1; biny < _capa.maxBinY; biny++) {
			info.addMode(Binning(binx, biny));
		}
	}
	info.pixelwidth(_capa.pixelSizeX * 1e-6);
	info.pixelheight(_capa.pixelSizeY * 1e-6);

	// exposure times
	if (_capa.supportsLongExposure) {
		info.maxexposuretime(3600);
		info.minexposuretime(0.2);
	} else {
		// this mainly concerns the Atik GP, which also has
		// a shorter minimum exposure time
		info.maxexposuretime(32 * _capa.maxShortExposure);
		info.minexposuretime(0.01);
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "add ccdinfo %s",
		info.toString().c_str());
	ccdinfo.push_back(info);

	CcdInfo	info8(ccdname(_camera, "8bit"), size, 0);
	for (unsigned int binx = 1; binx <= _capa.maxBinX; binx++) {
		for (unsigned int biny = 1; biny < _capa.maxBinY; biny++) {
			info8.addMode(Binning(binx, biny));
		}
	}
	info8.pixelwidth(_capa.pixelSizeX * 1e-6);
	info8.pixelheight(_capa.pixelSizeY * 1e-6);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "add ccdinfo %s",
		info8.toString().c_str());
	ccdinfo.push_back(info8);

	// cooler related stuff
	_tempSensorCount = _capa.tempSensorCount;
}

/**
 * \brief Destroy the Atik camera object
 */
AtikCamera::~AtikCamera() {
}

/**
 * \brief retrieve the last error from the camera
 */
std::string	AtikCamera::getLastError() {
	const char	*l = _camera->getLastError();
	if (l) {
		return std::string(l);
	}
	return std::string("(unknown error)");
}

/**
 * \brief get a AtikCcd object from the camera
 */
CcdPtr	AtikCamera::getCcd0(size_t ccdid) {
	if (ccdid >= ccdinfo.size()) {
		std::string	msg = stringprintf("ccd id %d out of range",
			ccdid);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::range_error(msg);
	}
	return CcdPtr(new AtikCcd(ccdinfo[ccdid], *this));
}

/**
 * \brief Find out how many ccds the camera has
 *
 * Note that 8bit mode is handled as a separate camera
 */
unsigned int	AtikCamera::nCcds() const {
	return (_capa.has8BitMode) ? 2 : 1;
}

/**
 * \brief Find out whether the camera has a filter wheel
 */
bool	AtikCamera::hasFilterWheel() const {
	return _capa.hasFilterWheel;
}

/**
 * \brief Get a filterwheel
 */
FilterWheelPtr	AtikCamera::getFilterWheel0() {
	debug(LOG_WARNING, DEBUG_LOG, 0, "the filter wheel is not implemented");
	return FilterWheelPtr(NULL);
}

/**
 * \brief Find out whether the camera has a guide port
 */
bool	AtikCamera::hasGuidePort() const {
	return _capa.hasGuidePort;
}

/**
 * \brief Get the AtikGuideport object from the camera
 */
GuidePortPtr	AtikCamera::getGuidePort0() {
	return GuidePortPtr(new AtikGuideport(*this));
}

/**
 * \brief Implement taking an image
 */
void	AtikCamera::exposureRun(Exposure& exposure, atik::AtikCcd& atikccd) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "run method of AtikCcd starting: %p",
		_camera);
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	ImagePtr	_image = ImagePtr(NULL);
	bool	rc;

	// flip the vertical offset
	int	xoffset = exposure.x();
	int	yoffset = atikccd.getInfo().size().height() - exposure.y()
				- exposure.height();
	ImagePoint	offset(xoffset, yoffset);

	// by default, try a short exposure
	bool	shortexposure = true;

	// set the shutter if we have
	if (_capa.hasShutter) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "setting shutter to %s",
			(exposure.needsshutteropen()) ? "open" : "closed");
		rc = _camera->setShutter(exposure.needsshutteropen());
		if (!rc) {
			std::string	msg = stringprintf("cannot set shutter:"
				" %s", getLastError().c_str());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s");
			throw std::runtime_error(msg);
		}
		shortexposure = exposure.needsshutteropen();
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "shutter set");

	// find out whether we are actually capable of performing the
	// exposure
	if ((!_capa.supportsLongExposure)
		&& (exposure.exposuretime() > _capa.maxShortExposure)) {
		std::string	msg = stringprintf("camera does not "
			"support long exposure, and exposure time %.3f "
			"exceeds limit %.3f for short exposures",
			exposure.exposuretime(), _capa.maxShortExposure);
		//exposure.exposuretime(_capa.maxShortExposure);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	}

	ImagePtr	resultimage(NULL);

	// now we have to decide whether we can do a long exposure
	try {
		if ((exposure.exposuretime() <= _capa.maxShortExposure)
			&& (shortexposure)) {
				resultimage = shortExposure(offset, exposure);
		} else if (!_capa.supportsLongExposure) {
			resultimage = multiExposure(offset, exposure);
		} else {
			resultimage = longExposure(offset, exposure);
		}
	} catch (const std::exception& x) {
		atikccd.updatestate(CcdState::exposed);
		throw x;
	}

	// save the resulting image
	_image = resultimage;
	atikccd.image(_image);
	atikccd.updatestate(CcdState::exposed);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "run method of AtikCcd complete");
}

/**
 * \brief do a short exposure
 */
ImagePtr	AtikCamera::shortExposure(const ImagePoint& offset,
			Exposure& exposure) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start a short exposure");

	bool rc = _camera->readCCD(offset.x(), offset.y(),
		exposure.width(), exposure.height(),
		exposure.mode().x(), exposure.mode().y(),
		exposure.exposuretime());
	if (!rc) {
		std::string	msg = stringprintf("cannot do "
			"exposure %.3f sec: %s",
			exposure.exposuretime(),
			getLastError().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	return getImage(exposure);
}

/**
 * \brief do a long exposure
 */
ImagePtr	AtikCamera::longExposure(const ImagePoint& offset,
			Exposure& exposure) {

	debug(LOG_DEBUG, DEBUG_LOG, 0, "start a long exposure");
	bool	rc = _camera->startExposure(false);
	if (!rc) {
		// cannot start exposure
		std::string	msg = stringprintf("cannot start long "
			"exposure: %s", getLastError().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	// wait for the exposure time to expire
	debug(LOG_DEBUG, DEBUG_LOG, 0, "wating for %.1fs for exposure",
		exposure.exposuretime());
	usleep(_camera->delay(exposure.exposuretime()));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "exposure time over, read CCD");
	rc = _camera->readCCD(offset.x(), offset.y(),
		exposure.width(), exposure.height(),
		exposure.mode().x(), exposure.mode().y());
	if (!rc) {
		std::string	msg = stringprintf("cannot read after "
			"long exposure: %s", getLastError().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	return getImage(exposure);
}

/**
 * \brief get an expousre
 */
ImagePtr	AtikCamera::getImage(Exposure& exposure) {
	// interpreting the data we have received
	debug(LOG_DEBUG, DEBUG_LOG, 0, "exposure %s complete. reading data",
		exposure.toString().c_str());
	ImageSize	size = exposure.frame().size() / exposure.mode();
	Image<unsigned short>	*image = new Image<unsigned short>(size);
	ImagePtr	resultimage(image);	// to make sure image is dealloc
	image->setOrigin(exposure.frame().origin());

	// set the mosaic type
	switch (_capa.colour) {
	case COLOUR_NONE:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "this is a mono camera");
		break;
	case COLOUR_RGGB:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "this is a GBRG color camera");
		// note that that when we flip the image later, we will get
		// an RGGB image for offset 0/0
		switch ((exposure.x() % 2) + 2 * (exposure.y() % 2)) {
		case 0:	image->setMosaicType(MosaicType::BAYER_GBRG);
			break;
		case 1:	image->setMosaicType(MosaicType::BAYER_BGGR);
			break;
		case 2:	image->setMosaicType(MosaicType::BAYER_RGGB);
			break;
		case 3:	image->setMosaicType(MosaicType::BAYER_GRBG);
			break;
		}
		break;
	default:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "unknown camera color type: %d",
			_capa.colour);
		break;
	}

	// now read the image data into the pixel array
	debug(LOG_DEBUG, DEBUG_LOG, 0, "reading image data: %d pixels",
		image->getSize().getPixels());
	bool	rc = _camera->getImage(image->pixels,
			image->getSize().getPixels());

	// throw an exception if we annot get the image
	if (!rc) {
		std::string	msg = stringprintf("cannot read data: %s",
			getLastError().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// we have to flip the image upside down, because our image class
	// has the origin in the lower left corner, while ATIK has it
	// in the upper left corner
	image->flip();

	return resultimage;
}

/**
 * \brief Multi exposure
 *
 * \param offset	
 * \param exposure
 */
ImagePtr	AtikCamera::multiExposure(const ImagePoint& offset,
		Exposure& exposure) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "starting multi exposure");
	double	accumulated = 0;

	// get a new exposure structure
	Exposure	shortexposure = exposure;
	shortexposure.exposuretime(_capa.maxShortExposure);

	// start building the result image
	ImageSize	size = exposure.frame().size() / exposure.mode();
	Image<unsigned short>	*image = new Image<unsigned short>(size);
	image->fill(0);

	// keep an ImagePtr for the image to ensure proper deallocation
	ImagePtr	resultimage(image);	// to make sure image is dealloc
	debug(LOG_DEBUG, DEBUG_LOG, 0, "accumulator %s",
		image->ImageBase::info().c_str());

	// add the exposure metadata
	exposure.addToImage(*image);

	// expose all the images and add them to the list
	int	counter = 0;
	double	remaining = exposure.exposuretime() - accumulated;
	while (remaining > 0.000001) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "start partial exposure %d",
			counter);
		ImagePtr	newimage = shortExposure(offset, shortexposure);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "got new image %s",
			newimage->info().c_str());

		// add the image
		Image<unsigned short>	*newimg
			= dynamic_cast<Image<unsigned short>*>(&*newimage);
		if (NULL == newimg) {
			throw std::runtime_error("not the right image type");
		}
		long	m = std::numeric_limits<unsigned short>::max();
		for (int x = 0; x < image->size().width(); x++) {
			for (int y = 0; y < image->size().height(); y++) {
				long	v = image->pixel(x, y);
				v += newimg->pixel(x, y);
				if (v >= m) {
					image->pixel(x, y) = m;
				} else {
					image->pixel(x, y) = v;
				}
			}
		}
		image->add(*newimg);
		accumulated += shortexposure.exposuretime();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "accumulated time: %.3f",
			accumulated);

		// if this is the first image, add all the metadata
		if (0 == counter) {
			image->setMosaicType(newimg->getMosaicType());
		}

		// compute the remaining exposure time
		remaining = exposure.exposuretime() - accumulated;
		if (remaining < shortexposure.exposuretime()) {
			shortexposure.exposuretime(remaining);
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "remaining time: %.3f",
			remaining);
		counter++;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "added %d images", counter);

	// return sum image
	return resultimage;
}

/**
 * \brief Abort the exposure
 */
void    AtikCamera::abortExposure() {
	_camera->abortExposure();
}

/**
 * \brief Ask the camera for the set temperature
 */
Temperature	AtikCamera::getSetTemperature(AtikCooler& cooler) {
	std::unique_lock<std::recursive_mutex>	lock(_mutex, std::try_to_lock);
	if (!lock) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot get set temperature");
		return cooler.Cooler::getSetTemperature();
	}
	COOLING_STATE	state;
	float	targetTemp = 0;
	float	power = 0;
	_camera->getCoolingStatus(&state, &targetTemp, &power);
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve cooler temperature: %.1f, "
	//	"power %.2f, state %d", targetTemp, power, state);
	if ((state == COOLING_ON) || (state == COOLING_SETPOINT)) {
		cooler.overrideSetTemperature(targetTemp + Temperature::zero);
	}
	return cooler.Cooler::getSetTemperature();
}

/**
 * \brief Get the actual temperature of the CCD
 */
Temperature	AtikCamera::getActualTemperature(AtikCooler& cooler) {
	std::unique_lock<std::recursive_mutex>	lock(_mutex, std::try_to_lock);
	if (!lock) {
		return Temperature(_last_actual_temperature);
	}
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve current Temp (%d)",
	//	_tempSensorCount);
	if (_tempSensorCount != 0) {
		float	currentTemp;
		_camera->getTemperatureSensorStatus(1, &currentTemp);
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "current Temp: %.1f",
		//	currentTemp);
		return _last_actual_temperature = currentTemp + Temperature::zero;
	};
	COOLING_STATE	state;
	float	targetTemp = 0;
	float	power = 0;
	_camera->getCoolingStatus(&state, &targetTemp, &power);
	switch (state) {
	case COOLING_ON:
	case COOLING_INACTIVE:
	case WARMING_UP:
		return _last_actual_temperature = Temperature::zero + 20;
	case COOLING_SETPOINT:
		return _last_actual_temperature
				= cooler.Cooler::getSetTemperature().temperature();
	}
	std::string	msg = stringprintf("unknown cooling state: %d",
		state);
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

/**
 * \brief Set the temperature of the CCD
 */
void	AtikCamera::setTemperature(const float temperature, AtikCooler& cooler) {
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set temperature: %f", temperature);
	cooler.overrideSetTemperature(temperature);
	if (isOn(cooler)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "setCooling(%f)",
			temperature - Temperature::zero);
		_camera->setCooling(temperature - Temperature::zero);
	}
}

/**
 * \brief find out whether the cooler is on
 */
bool	AtikCamera::isOn(AtikCooler& /* cooler */) {
	std::unique_lock<std::recursive_mutex>	lock(_mutex, std::try_to_lock);
	if (!lock) {
		return _is_on;
	}
	COOLING_STATE	state;
	float	targetTemp = 0;
	float	power = 0;
	_camera->getCoolingStatus(&state, &targetTemp, &power);
	return ((state == COOLING_ON) || (state == COOLING_SETPOINT));
}

/**
 * \brief turn the cooler on
 */
void	AtikCamera::setOn(bool onoff, AtikCooler& cooler) {
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	if (onoff) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "turn cooling on");
		_camera->setCooling(cooler.Cooler::getSetTemperature() - Temperature::zero);
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "turn cooling off");
		_camera->initiateWarmUp();
	}
	_is_on = onoff;
}

/**
 * \brief Start warming up the camera
 */
void	AtikCamera::initiateWarmUp() {
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	_camera->initiateWarmUp();
}

/**
 * \brief Get a user friendly name of the camera
 */
std::string	AtikCamera::userFriendlyName() const {
	if (_camera) {
		return std::string(_camera->getName());
	}
	return Device::userFriendlyName();
}

} // namespace atik
} // namespace camera
} // namespace astro
