/*
 * Qhy2Camera.cpp -- QHY camera implementation
 *
 * (c) 2020 Prof Dr Andreas Mueller, Hochschule Rapperswil
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

	// retrieve the handle
	_handle = Qhy2CameraLocator::handleForName(qhyname);

	// get CCD information

	// we can only work with CCDs that allow single frame mode
	if (QHYCCD_SUCCESS != IsQHYCCDControlAvailable(_handle,
		CAM_SINGLEFRAMEMODE)) {
		debug(LOG_WARNING, DEBUG_LOG, 0, "camera %s does not know "
			"single frame mode, no CCDs", qhyname.c_str());
		return;
	}

	// get pixel dimensions
	double	pixelwidth, pixelheight;
	{
		double	width, height;	// width 
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
		pixelwidth = pixelwidth / 1000000.;
		pixelheight = pixelheight / 1000000.;
		_totalsize = ImageSize(imagew, imageh);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "pixel dimensions: %.1fum x %.1fum",
		1000000. * pixelwidth, 1000000. * pixelheight);

	// get the effective area
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
		_effectivearea = ImageSize(sizeX, sizeY);
		_start = ImagePoint((int)startX, (int)startY);
		_offset = ImagePoint((int)startX,
				_totalsize.height() - startY - sizeY);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"effective image size: %s @ %s (start = %s)",
		_effectivearea.toString().c_str(), _offset.toString().c_str(),
		_start.toString().c_str());

	// get the available binning modes
	BinningSet	binningmodes;
	binningmodes.insert(Binning(1,1));
	if (QHYCCD_SUCCESS == IsQHYCCDControlAvailable(_handle,
		CAM_BIN2X2MODE)) {
		binningmodes.insert(Binning(2,2));
	}
	if (QHYCCD_SUCCESS == IsQHYCCDControlAvailable(_handle,
		CAM_BIN3X3MODE)) {
		binningmodes.insert(Binning(3,3));
	}
	if (QHYCCD_SUCCESS == IsQHYCCDControlAvailable(_handle,
		CAM_BIN4X4MODE)) {
		binningmodes.insert(Binning(4,4));
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "added %d binning modes",
		binningmodes.size());

	// find out whether the camera has a shutter
	bool	shutter = (QHYCCD_SUCCESS == IsQHYCCDControlAvailable(_handle,
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

	// read the readout modes and names
	_readoutmode_names = Qhy2CameraLocator::readmodelist(_handle);

	// find bit depth of the camera
	std::vector<int>	bitlist = Qhy2CameraLocator::bitlist(_handle);
	if (bitlist.size() > 0) {
		int	ccdindex = 0;
		for (auto bits : bitlist) {
			for (uint32_t mode = 0;
					mode < _readoutmode_names.size();
					mode++) {
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"creating %d bits ccd, read mode %d",
					bits, mode);
				CcdInfo	info = getinfo(mode, bits, ccdindex);
				info.addModes(binningmodes);
				info.shutter(shutter);
				info.pixelwidth(pixelwidth);
				info.pixelheight(pixelheight);
				info.minexposuretime(minexposuretime/1000000.);
				info.maxexposuretime(maxexposuretime/1000000.);
				ccdinfo.push_back(info);
				ccdindex++;
				debug(LOG_DEBUG, DEBUG_LOG, 0, "added CCD %s",
					info.toString(true).c_str());
			}
		}
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "creating default ccd");
		// just create the default CCDinfo
		int	ccdindex = 0;
		for (uint32_t mode = 0; mode < _readoutmode_names.size();
				mode++) {
			CcdInfo	info = getinfo(mode, 0, ccdindex);
			info.addModes(binningmodes);
			info.shutter(shutter);
			info.pixelwidth(pixelwidth);
			info.pixelheight(pixelheight);
			info.minexposuretime(minexposuretime / 1000000.);
			info.maxexposuretime(maxexposuretime / 1000000.);
			ccdindex++;
			ccdinfo.push_back(info);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "added CCD %s",
				info.toString(true).c_str());
		}
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
	if (QHYCCD_SUCCESS == IsQHYCCDControlAvailable(_handle,
		CONTROL_ST4PORT)) {
		GuidePortPtr	guideport(new Qhy2GuidePort(*this));
		return guideport;
	}
	throw Qhy2Error("guide port not present", -1);
}

/**
 * \brief Get the name of the readout mode from the mode number
 *
 * \param mode		the mode number used by the QHYCCD library
 */
std::string	Qhy2Camera::readoutmode(uint32_t mode) const {
	if (mode >= _readoutmode_names.size()) {
		std::string	msg = stringprintf("mode %d argument too large",
			mode);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw Qhy2Error(msg, -1);
	}
	return _readoutmode_names[mode];
}

/**
 * \brief Get the mode number from the name
 *
 * \param name		name of the readout mode
 */
uint32_t	Qhy2Camera::readoutmode(const std::string& name) const {
	for (uint32_t mode = 0; mode < _readoutmode_names.size(); mode++) {
		if (_readoutmode_names[mode] == name) {
			return mode;
		}
	}
	std::string	msg = stringprintf("readout mode '%s' not found",
		name.c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw Qhy2Error(msg, -1);
}

/**
 * \brief Get a CcdInfo object for a readout mode and bit depth
 *
 * \param mode		the readout mode number
 * \param bits		the pixel bit size
 * \param ccdindex	the index of the device
 */
CcdInfo	Qhy2Camera::getinfo(uint32_t mode, int bits, int ccdindex) {
	// build the name
	std::string	modename = readoutmode(mode);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating mode %s, %d bits ccd",
		modename.c_str(), bits);
	DeviceName	ccdname = name().child(DeviceName::Ccd, modename)
			.child(DeviceName::Ccd, stringprintf("%d", bits));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new ccd: %s",
		ccdname.toString().c_str());

	// retrieve the size
	uint32_t	width;
	uint32_t	height;
	int	rc = GetQHYCCDReadModeResolution(_handle, mode,
			&width, &height);
	if (QHYCCD_SUCCESS != rc) {
		std::string	msg = stringprintf("no resolution for mode %s",
			modename.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw Qhy2Error(msg, rc);
	}
	ImageSize	size(width, height);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "size for mode %s: %s", modename.c_str(),
		size.toString().c_str());

	// append the name based on the bit depth
	return CcdInfo(ccdname, size, ccdindex);
}

/**
 * \brief Retrieve the readout mode from the CCD info
 *
 * \param info		the CCD info
 */
uint32_t	Qhy2Camera::readoutmode(const CcdInfo& info) const {
	return readoutmode(info.name()[2]);
}

} // namespace qhy2
} // namespace camera
} // namespace astro
