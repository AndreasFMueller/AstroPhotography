/*
 * SbigCamera.cpp -- SBIG camera implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SbigCamera.h>
#include <SbigCcd.h>
#include <iostream>
#include <sbigudrv.h>
#include <utils.h>
#include <debug.h>

using namespace astro::camera;
using namespace astro::image;

namespace astro {
namespace camera {
namespace sbig {

SbigCamera::SbigCamera() {
	// XXX find out which USB number this is, we currently cannot really
	//     do this because the strings returned by the library are junk
	int	usbno = 0;

	// open the device
	OpenDeviceParams	openparams;
	openparams.deviceType = 0x7f02 + usbno;
	short	e = SBIGUnivDrvCommand(CC_OPEN_DEVICE, &openparams, NULL);
	if (e != CE_NO_ERROR) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot open device: %s",
			sbig_error(e).c_str());
		throw SbigError(e);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "device opened");

	// establish the link (it completely escapes me why this is a 
	// separate step from opening the device)
	EstablishLinkParams	establishparams;
	establishparams.sbigUseOnly = 0;
	EstablishLinkResults	results;
	e = SBIGUnivDrvCommand(CC_ESTABLISH_LINK, &establishparams, &results);
	if (e != CE_NO_ERROR) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot establish link: %s",
			sbig_error(e).c_str());
		throw SbigError(e);
	}
	cameraType = results.cameraType;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera type: %hu", cameraType);

	// we also get the camera type from the establish link command,
	// so we can now decide whether this is a camera with remote guide
	// head capability
	numberCcds = 0;
	switch (cameraType) {
	case STX_CAMERA:
	case STL_CAMERA:
		numberCcds = 3;
		break;
	default:
		numberCcds = 1;
		break;
	}

	// get the handle
	GetDriverHandleResults	driverhandle;
	e = SBIGUnivDrvCommand(CC_GET_DRIVER_HANDLE, NULL, &driverhandle);
	if (e != CE_NO_ERROR) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot get driver handle: %s",
			sbig_error(e).c_str());
		throw SbigError(e);
	}
	handle = driverhandle.handle;

	// query the driver info
	GetDriverInfoParams	driverinfoparams;
	GetDriverInfoResults0	driverinfo;
	for (driverinfoparams.request = 0; driverinfoparams.request < 3;
		driverinfoparams.request++) {
		e = SBIGUnivDrvCommand(CC_GET_DRIVER_INFO, &driverinfoparams,
			&driverinfo);
		if (e != CE_NO_ERROR) {
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot get driver info: %s",
				sbig_error(e).c_str());
		} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "driverinfo[%d]: %s ver %hu, "
			"maxrequest = %hu", driverinfoparams.request,
			driverinfo.name, driverinfo.version,
			driverinfo.maxRequest);
		}
	}

	// query the number of ccds this device has. All cameras always have
	// at least one CCD, the imaging CCD. Some cameras have a tracking
	// CCD, which we hope to detect using CC_GET_CCD_INFO. And there
	// are some cameras (STX and ST-L) which can have a remote guide
	// head, we detect this by testing for the camera type.
	GetCCDInfoParams	ccdinfoparams;
	GetCCDInfoResults0	ccdinfo;
	ccdinfoparams.request = CCD_INFO_TRACKING;
	e = SBIGUnivDrvCommand(CC_GET_CCD_INFO, &ccdinfoparams, &ccdinfo);
	if (e != CE_NO_ERROR) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no tracking ccd");
		numberCcds = 1;
	}
}

void	SbigCamera::sethandle() {
	SetDriverHandleParams	driverhandle;
	driverhandle.handle = handle;
	short	e = SBIGUnivDrvCommand(CC_SET_DRIVER_HANDLE, &driverhandle,
		NULL);
	if (e != CE_NO_ERROR) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot set driver handle: %s",
			sbig_error(e).c_str());
		throw SbigError(e);
	}
}

SbigCamera::~SbigCamera() {
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

CcdPtr	SbigCamera::getCcd(int id) {
	if ((id < 0) || (id >= numberCcds)) {
		throw std::range_error("ccd id not in range");
	}
	// get information about the size of this ccd
	GetCCDInfoParams	ccdinfoparams;
	GetCCDInfoResults0	ccdinfo;
	ccdinfoparams.request = id;
	short	e = SBIGUnivDrvCommand(CC_GET_CCD_INFO, &ccdinfoparams,
		&ccdinfo);
	if (e != CE_NO_ERROR) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot get CCD info: %s",
			sbig_error(e).c_str());
		throw SbigError(e);
	}

	// debugging: display all readout modes
	if (debuglevel >= LOG_DEBUG) {
		for (int i = 0; i < ccdinfo.readoutModes; i++) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "mode[%d] %hu x %hu", i,
				ccdinfo.readoutInfo[i].width,
				ccdinfo.readoutInfo[i].height);
		}
	}

	// now that we have he CCD info, we can create a ccd structure
	ImageSize	size(ccdinfo.readoutInfo[0].width,
				ccdinfo.readoutInfo[0].height);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating %d x %d ccd",
		size.width, size.height);

	return CcdPtr(new SbigCcd(size, id, *this));
}

} // namespace sbig
} // namespace camera
} // namespace astro
