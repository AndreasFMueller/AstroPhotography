/*
 * Qhy2Camera.cpp -- QHY camera implementation
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Qhy2Camera.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroExceptions.h>
#include <Qhy2Utils.h>
#include <Qhy2Ccd.h>
#include <Qhy2Locator.h>
#include <Qhy2GuidePort.h>

namespace astro {
namespace camera {
namespace qhy2 {

/**
 * \brief Construct a camera object
 *
 * \param qhyname	QHY the name of the device
 */
Qhy2Camera::Qhy2Camera(const std::string& qhyname)
	: astro::camera::Camera(Qhy2Name(qhyname).cameraname()) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "constructing camera '%s'",
		name().toString().c_str());

	_handle = Qhy2CameraLocator::handleForName(qhyname);

	// get CCD information

	// we can only work with CCDs that allow single frame mode
	if (QHYCCD_ERROR == IsQHYCCDControlAvailable(_handle,
		CAM_SINGLEFRAMEMODE)) {
		debug(LOG_WARNING, DEBUG_LOG, 0, "camera %s does not know "
			"single frame mode, no CCDs", qhyname.c_str());
		return;
	}

	// get the size
	ImageSize	size;
	{
		uint32_t	startX, startY, sizeX, sizeY;
		int	rc = GetQHYCCDEffectiveArea(_handle, &startX, &startY,
				&sizeX, &sizeY);
		if (rc != QHYCCD_SUCCESS) {
			std::string	msg = stringprintf("cannot get "
				"effective area from '%s'", qhyname.c_str());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s: %d", msg.c_str(), rc);
			throw Qhy2Error(msg, rc);
		}
		size = ImageSize(sizeX, sizeY);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image size: %s",
		size.toString().c_str());

	// get pixel dimensions
	double	pixelwidth, pixelheight;
	{
		double	width, height;
		uint32_t	imagew, imageh, bpp;
		int	rc = GetQHYCCDChipInfo(_handle, &width, &height,
				&imagew, &imageh,
				&pixelwidth, &pixelheight, &bpp);
		if (rc != QHYCCD_SUCCESS) {
			std::string	msg = stringprintf("cannot get pixel "
				"dimensions from '%s'", qhyname.c_str());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s: %d", msg.c_str(), rc);
			throw Qhy2Error(msg, rc);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "pixel dimensions: %.1fum x %.1fum",
		pixelwidth, pixelheight);

	// get the available binning modes
	BinningSet	binningmodes;
	binningmodes.insert(Binning(1,1));
	if (IsQHYCCDControlAvailable(_handle, CAM_BIN2X2MODE)) {
		binningmodes.insert(Binning(2,2));
	}
	if (IsQHYCCDControlAvailable(_handle, CAM_BIN3X3MODE)) {
		binningmodes.insert(Binning(3,3));
	}
	if (IsQHYCCDControlAvailable(_handle, CAM_BIN4X4MODE)) {
		binningmodes.insert(Binning(4,4));
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "added %d binning modes",
		binningmodes.size());

	// find out whether the camera has a shutter
	bool	shutter = (IsQHYCCDControlAvailable(_handle,
				CAM_MECHANICALSHUTTER)) ? true : false;
	int	rc = IsQHYCCDControlAvailable(_handle, CAM_COLOR);
	bool	color = (rc == BAYER_GB) || (rc == BAYER_GR)
			|| (rc == BAYER_BG) || (rc == BAYER_RG);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s is %sa color camera",
		qhyname.c_str(), (color) ? "" : "not ");

	// find the valid range of exposure times
	double	minexposuretime, maxexposuretime;
	{
		double	step;
		int	rc = GetQHYCCDParamMinMaxStep(_handle, CONTROL_EXPOSURE,
				&minexposuretime, &maxexposuretime, &step);
		if (rc != QHYCCD_SUCCESS) {
			std::string	msg = stringprintf("cannot get exposure"
				" times from '%s'", qhyname.c_str());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw Qhy2Error(msg, rc);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "exposure times between %.3fs and %.3fs",
		minexposuretime / 1000000., maxexposuretime / 1000000.);

	// find bit depth of the camera
	if (IsQHYCCDControlAvailable(_handle, CONTROL_TRANSFERBIT)) {
		double	min, max, step;
		int	rc = GetQHYCCDParamMinMaxStep(_handle,
				CONTROL_TRANSFERBIT, &min, &max, &step);
		if (rc != QHYCCD_SUCCESS) {
			std::string	msg = stringprintf("cannot get "
				"transfer range");
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw Qhy2Error(msg, rc);
		}
		int	bitstep = step;
		int	maxbits = max;
		int	ccdindex = 0;
		for (int bits = min; bits <= maxbits;
				bits += bitstep, ccdindex++) {
			CcdInfo	info(stringprintf("%d", bits), size, ccdindex);
			info.addModes(binningmodes);
			info.shutter(shutter);
			info.pixelwidth(pixelwidth);
			info.pixelheight(pixelheight);
			info.minexposuretime(minexposuretime / 1000000.);
			info.maxexposuretime(maxexposuretime / 1000000.);
			ccdinfo.push_back(info);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "added CCD %s",
				info.toString(true).c_str());
		}
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "creating default ccd");
		// just create the default CCDinfo
		CcdInfo	defaultinfo(CcdInfo::defaultname(name(), "ccd"),
			size, 0);
		defaultinfo.addModes(binningmodes);
		defaultinfo.shutter(shutter);
		defaultinfo.pixelwidth(pixelwidth);
		defaultinfo.pixelheight(pixelheight);
		defaultinfo.minexposuretime(minexposuretime / 1000000.);
		defaultinfo.maxexposuretime(maxexposuretime / 1000000.);
		ccdinfo.push_back(defaultinfo);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "added CCD %s",
			defaultinfo.toString(true).c_str());
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera with %d CCDs created",
		ccdinfo.size());
}

/**
 * \brief Destroy a camera
 */
Qhy2Camera::~Qhy2Camera() {
}

/**
 * \brief Get the qhyname from the camera
 */
std::string	Qhy2Camera::qhyname() const {
	return name()[1];
}

/**
 * \brief Get the Ccd
 */
CcdPtr	Qhy2Camera::getCcd0(size_t ccdindex) {
	if (ccdindex > ccdinfo.size()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "CCD index %u out of range",
			ccdindex);
		throw NotFound("ccd id out of range");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "find ccd %d", ccdindex);
	CcdPtr	ccd(new Qhy2Ccd(ccdinfo[ccdindex], *this));
	return ccd;
}

/**
 * \brief Get the guide port
 */
GuidePortPtr	Qhy2Camera::getGuidePort0() {
	if (IsQHYCCDControlAvailable(_handle, CONTROL_ST4PORT)) {
		GuidePortPtr	guideport(new Qhy2GuidePort(*this));
		return guideport;
	}
	throw Qhy2Error("guide port not present", -1);
}

} // namespace qhy2
} // namespace camera
} // namespace astro