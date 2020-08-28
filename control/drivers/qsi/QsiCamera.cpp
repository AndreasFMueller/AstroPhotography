/*
 * QsiCamera.cpp --
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <QsiCamera.h>
#include <QsiCcd.h>
#include <QsiFilterWheel.h>
#include <QsiGuidePort.h>

namespace astro {
namespace camera {
namespace qsi {

/**
 * \brief Construct a QSI camera object
 */
QsiCamera::QsiCamera(const std::string& _name) : Camera(_name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "constructing camera %s at %p",
		_name.c_str(), this);
	std::unique_lock<std::recursive_mutex>	lock(mutex);
	try {
		DeviceName	devname(name());
		camera().put_UseStructuredExceptions(true);
		camera().put_SelectCamera(devname.unitname());
	} catch (...) {
		throw std::runtime_error("cannot select camera");
	}

	// connect to the camera
	try {
		_camera.put_Connected(true);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot open connection: %s",
			x.what());
		throw x;
	}

	// get the name
	_camera.get_Name(_userFriendlyName);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera name: %s",
		_userFriendlyName.c_str());

	// get the filterwheel and guideport information
	camera().get_HasFilterWheel(&_hasfilterwheel);
	camera().get_CanPulseGuide(&_hasguideport);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "features: %s filterwheel, %s guideport",
		(_hasfilterwheel) ? "has" : "no",
		(_hasguideport) ? "has" : "no");

	// query the information of the CCD
	long	xsize, ysize;
	camera().get_CameraXSize(&xsize);
	camera().get_CameraYSize(&ysize);
	DeviceName	ccdname(name(), DeviceName::Ccd);
	CcdInfo	info(ccdname, astro::image::ImageSize(xsize, ysize), 0);

	// get pixel dimensions
	double	pixelsize;
	camera().get_PixelSizeX(&pixelsize);
	info.pixelwidth(pixelsize / 1000000.);
	camera().get_PixelSizeY(&pixelsize);
	info.pixelheight(pixelsize / 1000000.);

	// get the exposure time limits
	double	minexposuretime;
	camera().get_MinExposureTime(&minexposuretime);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "min exposure: %.25f", minexposuretime);
	info.minexposuretime(minexposuretime);
	double	maxexposuretime;
	camera().get_MaxExposureTime(&maxexposuretime);
	info.maxexposuretime(maxexposuretime);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "info: %s", info.toString().c_str());

	// get the binning modes
	bool	p2bin;
	camera().get_PowerOfTwoBinning(&p2bin);
	bool	asymbin;
	camera().get_CanAsymmetricBin(&asymbin);
	short	maxbinx, maxbiny;
	camera().get_MaxBinX(&maxbinx);
	camera().get_MaxBinY(&maxbiny);
	if (p2bin) {
		for (int xbin = 1; xbin <= maxbinx; xbin <<= 1) {
			for (int ybin = 1; ybin <= maxbiny; ybin <<= 1) {
				if (asymbin || (xbin == ybin)) {
					info.addMode(Binning(xbin, ybin));
				}
			}
		}
	} else {
		for (int xbin = 1; xbin <= maxbinx; xbin++) {
			for (int ybin = 1; ybin <= maxbiny; ybin++) {
				if (asymbin || (xbin == ybin)) {
					info.addMode(Binning(xbin, ybin));
				}
			}
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got %d binning modes",
		info.modes().size());

	// find out whether the camera has a shutter
	bool	hasshutter = false;
	camera().get_HasShutter(&hasshutter);
	info.shutter(hasshutter);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s shutter",
		(hasshutter) ? "has" : "no");

	// add the ccdinfo to the 
	ccdinfo.push_back(info);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera %p construction complete", this);
}

/**
 * \brief Destroy the QSI camera object
 */
QsiCamera::~QsiCamera() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "disconnect the camera");

	if (_ccd) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "stopping CCD");
		try {
			QsiCcd	*qsiccd = dynamic_cast<QsiCcd*>(&*_ccd);
			if (NULL != qsiccd) {
				qsiccd->stop();
			}
		} catch (const std::exception& x) {
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot stop ccd: %s",
				x.what());
		} catch (...) {
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot stop ccd");
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "CCD stopped");
	}

	if (_filterwheel) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "stopping filterwheel");
		try {
			QsiFilterWheel	*fw = dynamic_cast<QsiFilterWheel*>(&*_filterwheel);
			if (fw) {
				fw->threadwait();
			}
		} catch (const std::exception& x) {
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot stop filterwheel:"
				" %s", x.what());
		} catch (...) {
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot stop filterwheel");
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "filterwheel stopped");
	}

	// now we can turn of the camera
	std::unique_lock<std::recursive_mutex>	lock(mutex);
	camera().put_Connected(false);
}

/**
 * \brief Perform a camera reset on the QSI camera
 */
void	QsiCamera::reset() {
	// XXX perform a camera reset
}

/**
 * \brief Get the CCD from the camera
 */
CcdPtr	QsiCamera::getCcd0(size_t id) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get CCD %d from %p", id, this);
	std::unique_lock<std::recursive_mutex>	lock(mutex);
	if (id > 0) {
		throw std::invalid_argument("only CCD 0 defined");
	}
	_ccd = CcdPtr(new QsiCcd(ccdinfo[0], *this));
	return _ccd;
}

/**
 * \brief Find out whether the camera has a filter wheel
 */
bool	QsiCamera::hasFilterWheel() const {
	return _hasfilterwheel;
}

/**
 * \brief Get the Filter wheel
 */
FilterWheelPtr	QsiCamera::getFilterWheel0() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get the filterwheel from %p", this);
	std::unique_lock<std::recursive_mutex>	lock(mutex);
	if (!_hasfilterwheel) {
		throw std::invalid_argument("camera has no filter wheel");
	}
	_filterwheel = FilterWheelPtr(new QsiFilterWheel(*this));
	return _filterwheel;
}

/**
 * \brief Check whether the camera has a guider port
 */
bool	QsiCamera::hasGuidePort() const {
	return _hasguideport;
}

/**
 * \brief Get the Guider port
 */
GuidePortPtr	QsiCamera::getGuidePort0() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get the guideport");
	std::unique_lock<std::recursive_mutex>	lock(mutex);
	if (!_hasguideport) {
		throw std::runtime_error("camera has no guider port");
	}
	return GuidePortPtr(new QsiGuidePort(*this));
}

/**
 * \brief Find out whether the camera is color
 */
bool	QsiCamera::isColor() const {
	// XXX dummy implementation, only monochrome chips
	return false;
}

} // namespace qsi
} // namespace camera
} // namespace astro
