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
	} else {
		info.maxexposuretime(_capa.maxShortExposure);
	}
	info.minexposuretime(0.2);

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

AtikCamera::~AtikCamera() {
}

std::string	AtikCamera::getLastError() {
	const char	*l = _camera->getLastError();
	if (l) {
		return std::string(l);
	}
	return std::string("(unknown error)");
}

CcdPtr	AtikCamera::getCcd0(size_t ccdid) {
	if (ccdid >= ccdinfo.size()) {
		std::string	msg = stringprintf("ccd id %d out of range",
			ccdid);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::range_error(msg);
	}
	return CcdPtr(new AtikCcd(ccdinfo[ccdid], *this));
}

unsigned int	AtikCamera::nCcds() const {
	return (_capa.has8BitMode) ? 2 : 1;
}

bool	AtikCamera::hasFilterWheel() const {
	return _capa.hasFilterWheel;
}

FilterWheelPtr	AtikCamera::getFilterWheel0() {
	return FilterWheelPtr(NULL);
}

bool	AtikCamera::hasGuidePort() const {
	return _capa.hasGuidePort;
}

GuidePortPtr	AtikCamera::getGuidePort0() {
	return GuidePortPtr(new AtikGuideport(*this));
}

void	AtikCamera::exposureRun(Exposure& exposure, atik::AtikCcd& atikccd) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "run method of AtikCcd starting: %p",
		_camera);
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	ImagePtr	_image = ImagePtr(NULL);
	bool	rc;

	// set the shutter if we have
	bool	shortexposure = true;
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
		exposure.exposuretime(_capa.maxShortExposure);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	}

	// now we have to decide whether we can do a long exposure
	if ((exposure.exposuretime() <= _capa.maxShortExposure)
		&& (shortexposure)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "start a short exposure");
		// do a short exposure
		rc = _camera->readCCD(exposure.x(), exposure.y(),
			exposure.width(), exposure.height(),
			exposure.mode().x(), exposure.mode().y(),
			exposure.exposuretime());
		if (!rc) {
			atikccd.updatestate(CcdState::exposed);
			std::string	msg = stringprintf("cannot do "
				"exposure %.3f sec: %s",
				exposure.exposuretime(),
				getLastError().c_str());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "start a long exposure");
		rc = _camera->startExposure(false);
		if (!rc) {
			// cannot start exposure
			std::string	msg = stringprintf("cannot start long "
				"exposure: %s", getLastError().c_str());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
		// wait for the exposure time to expire
		usleep(_camera->delay(exposure.exposuretime()));
		rc = _camera->readCCD(exposure.x(), exposure.y(),
			exposure.width(), exposure.height(),
			exposure.mode().x(), exposure.mode().y());
		if (!rc) {
			atikccd.updatestate(CcdState::exposed);
			std::string	msg = stringprintf("cannot read after "
				"long exposure: %s", getLastError().c_str());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
	}

	// interpreting the data we have received
	debug(LOG_DEBUG, DEBUG_LOG, 0, "exposure %s complete. reading rata",
		exposure.toString().c_str());
	ImageSize	size = exposure.frame().size() / exposure.mode();
	Image<unsigned short>	*image = new Image<unsigned short>(size);
	image->setOrigin(exposure.frame().origin());
	switch (_capa.colour) {
	case COLOUR_NONE:
	case COLOUR_RGGB:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "this is a RGGB color camera");
		image->setMosaicType(MosaicType::BAYER_RGGB);
		break;
	default:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "unknown camera color type: %d",
			_capa.colour);
		break;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "reading image data: %d pixels",
		image->getSize().getPixels());
	rc = _camera->getImage(image->pixels, image->getSize().getPixels());
	if (!rc) {
		atikccd.updatestate(CcdState::exposed);
		std::string	msg = stringprintf("cannot read data: %s",
			getLastError().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "reading image data complete");
	_image = ImagePtr(image);
	atikccd.image(_image);
	atikccd.updatestate(CcdState::exposed);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "run method of AtikCcd complete");
}

void    AtikCamera::abortExposure() {
	_camera->abortExposure();
}

float	AtikCamera::getSetTemperature(AtikCooler& cooler) {
	std::unique_lock<std::recursive_mutex>	lock(_mutex, std::defer_lock);
	if (!lock.try_lock()) {
		std::string	msg("cannot lock in getSetTemperatur()");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	COOLING_STATE	state;
	float	targetTemp = 0;
	float	power = 0;
	_camera->getCoolingStatus(&state, &targetTemp, &power);
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve cooler temperature: %.1f, "
	//	"power %.2f, state %d", targetTemp, power, state);
	if ((state == COOLING_ON) || (state == COOLING_SETPOINT)) {
		cooler.Cooler::setTemperature(targetTemp + 273.15);
	}
	return cooler.Cooler::getSetTemperature();
}

float	AtikCamera::getActualTemperature(AtikCooler& cooler) {
	std::unique_lock<std::recursive_mutex>	lock(_mutex, std::defer_lock);
	if (!lock.try_lock()) {
		std::string	msg("cannot lock in getActualTemperature()");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve current Temp (%d)",
	//	_tempSensorCount);
	if (_tempSensorCount != 0) {
		float	currentTemp;
		_camera->getTemperatureSensorStatus(1, &currentTemp);
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "current Temp: %.1f",
		//	currentTemp);
		return currentTemp + 273.15;
	};
	COOLING_STATE	state;
	float	targetTemp = 0;
	float	power = 0;
	_camera->getCoolingStatus(&state, &targetTemp, &power);
	switch (state) {
	case COOLING_ON:
	case COOLING_INACTIVE:
	case WARMING_UP:
		return 273.15 + 20;
	case COOLING_SETPOINT:
		return cooler.Cooler::getSetTemperature();
	}
	std::string	msg = stringprintf("unknown cooling state: %d",
		state);
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

void	AtikCamera::setTemperature(const float temperature, AtikCooler& cooler) {
	std::unique_lock<std::recursive_mutex>	lock(_mutex, std::defer_lock);
	if (lock.try_lock()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "locked");
	} else {
		std::string	msg("cannot lock in setTemperature()");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set temperature: %f", temperature);
	cooler.Cooler::setTemperature(temperature);
	if (isOn(cooler)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "setCooling(%f)",
			temperature - 273.15);
		_camera->setCooling(temperature - 273.15);
	}
}

bool	AtikCamera::isOn(AtikCooler& /* cooler */) {
	std::unique_lock<std::recursive_mutex>	lock(_mutex, std::defer_lock);
	if (!lock.try_lock()) {
		std::string	msg("cannot lock in isOn()");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	COOLING_STATE	state;
	float	targetTemp = 0;
	float	power = 0;
	_camera->getCoolingStatus(&state, &targetTemp, &power);
	return ((state == COOLING_ON) || (state == COOLING_SETPOINT));
}

void	AtikCamera::setOn(bool onoff, AtikCooler& cooler) {
	std::unique_lock<std::recursive_mutex>	lock(_mutex, std::defer_lock);
	if (lock.try_lock()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "locked");
	} else {
		std::string	msg("cannot lock in setOn()");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	if (onoff) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "turn cooling on");
		_camera->setCooling(cooler.Cooler::getSetTemperature() - 273.15);
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "turn cooling off");
		_camera->initiateWarmUp();
	}
}

void	AtikCamera::initiateWarmUp() {
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	_camera->initiateWarmUp();
}

} // namespace atik
} // namespace camera
} // namespace astro
