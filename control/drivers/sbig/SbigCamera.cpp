/*
 * SbigCamera.cpp -- SBIG camera implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_LPARDRV_H
#include <lpardrv.h>
#else
#ifdef HAVE_SBIGUDRV_LPARDRV_H
#include <SBIGUDrv/lpardrv.h>
#endif /* HAVE_SBIGUDRV_LPARDRV_H */
#endif

#include <SbigLock.h>
#include <SbigLocator.h>
#include <SbigCamera.h>
#include <SbigCcd.h>
#include <iostream>
#include <utils.h>
#include <AstroDebug.h>
#include <SbigFilterWheel.h>
#include <SbigGuidePort.h>
#include <AstroFormat.h>

using namespace astro::camera;
using namespace astro::image;

namespace astro {
namespace camera {
namespace sbig {

/**
 * \brief Pixel size conversion
 *
 * SBIG cameras specify the pixel size in BCD format, which somewhat
 * annoying to convert to. This method performs the conversion from
 * XXXXXX.XX fixed point BCD format in micrometers to a float in milimeters.
 */
static float	pixelsize(unsigned long sbigsize) {
	double	result = 0;
	double	multiplier = 0.00000001;
	while (sbigsize) {
		int	n = sbigsize & 0xf;
		result = result + multiplier * n;
		sbigsize >>= 4;
		multiplier *= 10;
	}
	return result;
}

static DeviceName	cameraname(int usbno) {
	return DeviceName(std::string("camera:sbig/") + std::to_string(usbno));
}

short	SbigCamera::current_handle = -1;

void	SbigCamera::query_usb(QueryUSBResults *results) {
	SbigLock	lock;
	short   e = SBIGUnivDrvCommand(CC_QUERY_USB, NULL, results);
        if (e != CE_NO_ERROR) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot get camera list: %s",
			sbig_error(e).c_str());
		throw SbigError(e);
        }
}

void	SbigCamera::open_device(int usbno) {
	SbigLock	lock;
	OpenDeviceParams	openparams;
	openparams.deviceType = 0x7f02 + usbno;
	short	e = SBIGUnivDrvCommand(CC_OPEN_DEVICE, &openparams, NULL);
	if (e != CE_NO_ERROR) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot open device: %s",
			sbig_error(e).c_str());
		throw SbigError(e);
	}
}

unsigned short	SbigCamera::get_camera_type() {
	SbigLock	lock;
	EstablishLinkParams	establishparams;
	establishparams.sbigUseOnly = 0;
	EstablishLinkResults	establishresults;
	short	e = SBIGUnivDrvCommand(CC_ESTABLISH_LINK, &establishparams,
		&establishresults);
	if (e != CE_NO_ERROR) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot establish link: %s",
			sbig_error(e).c_str());
		throw SbigError(e);
	}
	return establishresults.cameraType;
}

short	SbigCamera::get_driver_handle() {
	SbigLock	lock;
	GetDriverHandleResults	driverhandle;
	short	e = SBIGUnivDrvCommand(CC_GET_DRIVER_HANDLE, NULL,
			&driverhandle);
	if (e != CE_NO_ERROR) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot get driver handle: %s",
			sbig_error(e).c_str());
		throw SbigError(e);
	}
	return driverhandle.handle;
}

void	SbigCamera::close_device() {
	SbigLock	lock;
	// set the handle first
	sethandle();

	// close the device
	short	e = SBIGUnivDrvCommand(CC_CLOSE_DEVICE, NULL, NULL);
	if (e != CE_NO_ERROR) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot close device: %s",
			sbig_error(e).c_str());
		// throw SbigError(e);
	}
}

/**
 * \brief Open the SBIG UDRV library
 *
 * \param usbno   USB number of the camera.
 */
SbigCamera::SbigCamera(int usbno) : Camera(cameraname(usbno)) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating SBIG camera object %d", usbno);

	SbigLock	sbiglock;

	// make sure we can really find this camera, and construct the name
	// of the camera
	QueryUSBResults results;
	query_usb(&results);
	if ((usbno >= results.camerasFound) ||
		(!results.usbInfo[usbno].cameraFound)) {
		std::string	msg = stringprintf("camera %d not found",
			usbno);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	_name = stringprintf("camera:sbig/%s",
		results.usbInfo[usbno].serialNumber);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "device name: %s",
		_name.toString().c_str());

	// open the device
	open_device(usbno);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "device opened");

	// establish the link (it completely escapes me why this is a 
	// separate step from opening the device)
	cameraType = get_camera_type();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera type: %hu", cameraType);

	// get the handle
	handle = get_driver_handle();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got driver handle %d", handle);

#if 0
	// query the driver info
	GetDriverInfoParams	driverinfoparams;
	GetDriverInfoResults0	driverinfo;
	for (driverinfoparams.request = 0; driverinfoparams.request < 3;
		driverinfoparams.request++) {
		short	e = SBIGUnivDrvCommand(CC_GET_DRIVER_INFO, &driverinfoparams,
			&driverinfo);
		if (e != CE_NO_ERROR) {
			debug(LOG_ERR, DEBUG_LOG, 0,
				"cannot get driver info for %d: %s",
				driverinfoparams.request,
				sbig_error(e).c_str());
		} else {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "driverinfo[%d]: %s "
				"ver %hu, maxrequest = %hu",
				driverinfoparams.request,
				driverinfo.name, driverinfo.version,
				driverinfo.maxRequest);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "driver info retrieved");
#endif

	// we want to assign CCD ids sequentially
	unsigned int	ccdidcounter = 0;

	// imaging ccd
	debug(LOG_DEBUG, DEBUG_LOG, 0, "query imaging CCD info");
	try {
		CcdInfo	ccd = get_ccd_info(CCD_INFO_IMAGING, "Imaging",
			ccdidcounter);
		ccd.shutter(true);
		ccdinfo.push_back(ccd);
		ccdidcounter++;
	} catch (...) {
		// ignore
	}
		
	// tracking ccd if present
	debug(LOG_DEBUG, DEBUG_LOG, 0, "querying tracking CCD info");
	try {
		CcdInfo	ccd = get_ccd_info(CCD_INFO_TRACKING, "Tracking",
			ccdidcounter);
		ccdinfo.push_back(ccd);
		ccdidcounter++;
	} catch (...) {
		// ignore
	}

	// external tracking ccd, if present
	debug(LOG_DEBUG, DEBUG_LOG, 0, "querying external tracking CCD info");
	try {
		CcdInfo	ccd = get_ccd_info(CCD_INFO_EXTENDED,
			"external Tracking", ccdidcounter);
		ccdinfo.push_back(ccd);
		ccdidcounter++;
	} catch (...) {
		// ignore
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera constructor complete");
}

/**
 * \brief Common method to retrieve information about a CCD
 *
 * \param request	SBIG driver library request code for this CCD
 * \param basename	the name we want to give this CCD in the URL naming
 * \param ccdindex	the index of this CCD in the array of CCDs
 */
CcdInfo	SbigCamera::get_ccd_info(unsigned short request,
		const std::string& basename, unsigned int ccdindex) {
	GetCCDInfoParams	ccdinfoparams;
	GetCCDInfoResults0	ccdinforesult;
	ccdinfoparams.request = request;
	short	e = SBIGUnivDrvCommand(CC_GET_CCD_INFO, &ccdinfoparams,
			&ccdinforesult);
	if (e != CE_NO_ERROR) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no imaging ccd");
		throw SbigError(e);
	} else {
		// here we assume that the largest readout mode is
		// delivered first, otherwise we would have to scan
		// the readout modes for one with mode == 0 (RM_1X1)
		ImageSize	ccdsize(ccdinforesult.readoutInfo[0].width,
			ccdinforesult.readoutInfo[0].height);
		DeviceName	ccdname(name(), DeviceName::Ccd, basename);
		CcdInfo	ccd(ccdname, ccdsize, ccdindex);
		long	w = ccdinforesult.readoutInfo[0].pixelWidth;
		long	h = ccdinforesult.readoutInfo[0].pixelHeight;
		ccd.pixelwidth(pixelsize(w));
		ccd.pixelheight(pixelsize(h));
		ccd.shutter(true);

		debug(LOG_DEBUG, DEBUG_LOG, 0, "found imageing ccd: %s",
			ccd.toString().c_str());
		for (int i = 0; i < ccdinforesult.readoutModes; i++) {
			SbigBinningAdd(ccd, ccdinforesult.readoutInfo[i].mode);
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"mode[%d]: %d x %d (%04x)",
				i, 
				ccdinforesult.readoutInfo[i].width,
				ccdinforesult.readoutInfo[i].height,
				ccdinforesult.readoutInfo[i].mode);
		}
		ccd.shutter(true);
		return ccd;
	}
}

/**
 * \brief Set the handle of the current camera
 *
 * The  SBIG universal driver library keeps track of the camera to talk to
 * via a handle. However, handling this handle is really awkward. This method
 * helps ensuring that whenever a camera operation is attempted, the 
 * handle is set correctly.
 *
 * XXX There are some concurrency issues here: we should really make sure 
 * that now function is attempted on a camera while an uninterruptible
 * operation on some other camera is in progress. But then it should
 * really be the driver library's task to ensure such basic stuff.
 */
void	SbigCamera::sethandle() {
	SbigLock	lock;

	// nothing needs to be done if the handle is already set correctly
	// not that we need to do this while locked, because otherwise
	// the handle might be stolen by another thread
	if (handle == current_handle) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "handle setting not necessary");
		return;
	}

	// setting the handle, but this is only needed if the driver handle
	// is different from our handle
	SetDriverHandleParams	driverhandle;
	driverhandle.handle = handle;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setting handle from %hd to %hd",
		current_handle, handle);
	short	e = SBIGUnivDrvCommand(CC_SET_DRIVER_HANDLE, &driverhandle,
		NULL);
	if (e != CE_NO_ERROR) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot set driver handle: %s",
			sbig_error(e).c_str());
		throw SbigError(e);
	}
	current_handle = handle;
}

/**
 * \brief Destroy the SBIG camera.
 *
 * This cleans up the handle of the camera and closes the device.
 */
SbigCamera::~SbigCamera() {
	close_device();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera %hd is being destructed",
		handle);
}

/**
 * \brief Get a CCD from an SBIG camera.
 *
 * \param id     ID of the CCD
 */
CcdPtr	SbigCamera::getCcd0(size_t id) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get ccd %u (of %d)", id,
		ccdinfo.size());
	if (id >= ccdinfo.size()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "ccd %d not in range", id);
		throw std::range_error("ccd id not in range");
	}

	CcdInfo	ccd = ccdinfo[id];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ccd: %s", ccd.toString().c_str());

	// now that we have he CCD info, we can create a ccd structure
	SbigCcd	*sbigccd = new SbigCcd(ccd, id, *this);
	CcdPtr	result(sbigccd);

	// ST-i is the only camera without a cooler, and only the imager
	// ccd can have a cooler
	if (id == 0) {
		switch (cameraType) {
		case STI_CAMERA:
			sbigccd->setCooler(false);
			break;
		default:
			break;
		}
	}

	// that's it
	debug(LOG_DEBUG, DEBUG_LOG, 0, "returning CCD");
	return result;
}

/**
 * \brief find out whether the camera has a filter wheel
 */
bool	SbigCamera::hasFilterWheel() const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "does this camera have a filter wheel?");
	switch (cameraType) {
	case STX_CAMERA:
		return true;
	}
	// XXX that's not quite correct ;-)
	return false;
}

/**
 * \brief Get the FilterWheel object
 *
 * If the camera has a filter wheel, this method returns a filter wheel
 * object which allows to control the filter wheel position.
 */
FilterWheelPtr	SbigCamera::getFilterWheel0() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get the filter wheel");
	return FilterWheelPtr(new SbigFilterWheel(*this));
}

/**
 * \brief find out whether the camera has a guider port
 */
bool	SbigCamera::hasGuidePort() const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "does this camera have a guider port?");
	// XXX that's not quite correct ;-)
	return true;
}

/**
 * \brief Get the Guider Port object
 *
 * If the camera has a guider port, thie object allows to retrieve a
 * GuidePort object to control the guider port.
 */
GuidePortPtr	SbigCamera::getGuidePort0() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get the guider port");
	return GuidePortPtr(new SbigGuidePort(*this));
}

} // namespace sbig
} // namespace camera
} // namespace astro
