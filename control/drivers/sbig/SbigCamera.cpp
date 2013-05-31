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
#include <SbigFilterWheel.h>
#include <SbigGuiderPort.h>

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

	// get the handle
	GetDriverHandleResults	driverhandle;
	e = SBIGUnivDrvCommand(CC_GET_DRIVER_HANDLE, NULL, &driverhandle);
	if (e != CE_NO_ERROR) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot get driver handle: %s",
			sbig_error(e).c_str());
		throw SbigError(e);
	}
	handle = driverhandle.handle;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got driver handle");

	// query the driver info
	GetDriverInfoParams	driverinfoparams;
	GetDriverInfoResults0	driverinfo;
	for (driverinfoparams.request = 0; driverinfoparams.request < 3;
		driverinfoparams.request++) {
		e = SBIGUnivDrvCommand(CC_GET_DRIVER_INFO, &driverinfoparams,
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

	// we now get the ccd info for all ccds of this camera
	GetCCDInfoParams	ccdinfoparams;
	GetCCDInfoResults0	ccdinforesult;

	// we want to assign CCD ids sequentially
	unsigned int	ccdidcounter = 0;

	// imaging ccd
	debug(LOG_DEBUG, DEBUG_LOG, 0, "query imaging CCD info");
	ccdinfoparams.request = CCD_INFO_IMAGING;
	e = SBIGUnivDrvCommand(CC_GET_CCD_INFO, &ccdinfoparams, &ccdinforesult);
	if (e != CE_NO_ERROR) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no imaging ccd");
	} else {
		CcdInfo	ccd;
		ccd.ccdid = ccdidcounter++;
		ccd.name = std::string("Imaging");
		// here we assume that the largest readout mode is
		// delivered first, otherwise we would have to scan
		// the readout modes for one with mode == 0 (RM_1X1)
		ccd.size = ImageSize(ccdinforesult.readoutInfo[0].width,
			ccdinforesult.readoutInfo[0].height);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found imageing ccd: %s",
			ccd.toString().c_str());
		for (int i = 0; i < ccdinforesult.readoutModes; i++) {
			ccd.binningmodes.insert(SbigMode2Binning(
				ccdinforesult.readoutInfo[i].mode));
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"mode[%d]: %d x %d (%04x)",
				i, 
				ccdinforesult.readoutInfo[i].width,
				ccdinforesult.readoutInfo[i].height,
				ccdinforesult.readoutInfo[i].mode);
		}

		ccdinfo.push_back(ccd);
	}

	// tracking ccd if present
	debug(LOG_DEBUG, DEBUG_LOG, 0, "querying tracking CCD info");
	ccdinfoparams.request = CCD_INFO_TRACKING;
	e = SBIGUnivDrvCommand(CC_GET_CCD_INFO, &ccdinfoparams, &ccdinforesult);
	if (e != CE_NO_ERROR) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no tracking ccd");
	} else {
		CcdInfo	ccd;
		ccd.ccdid = ccdidcounter++;
		ccd.name = std::string("Tracking");
		ccd.size = ImageSize(ccdinforesult.readoutInfo[0].width,
			ccdinforesult.readoutInfo[0].height);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found tracking ccd: %s",
			ccd.toString().c_str());

		for (int i = 0; i < ccdinforesult.readoutModes; i++) {
			ccd.binningmodes.insert(SbigMode2Binning(
				ccdinforesult.readoutInfo[i].mode));
			debug(LOG_DEBUG, DEBUG_LOG, 0, "mode[%d]: %d x %d",
				i, 
				ccdinforesult.readoutInfo[i].width,
				ccdinforesult.readoutInfo[i].height,
				ccdinforesult.readoutInfo[i].mode);
		}
		ccdinfo.push_back(ccd);
	}

	// external tracking ccd, if present
	debug(LOG_DEBUG, DEBUG_LOG, 0, "querying external tracking CCD info");
	ccdinfoparams.request = CCD_INFO_TRACKING;
	e = SBIGUnivDrvCommand(CC_GET_CCD_INFO, &ccdinfoparams, &ccdinforesult);
	if (e != CE_NO_ERROR) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no external tracking ccd");
	} else {
		CcdInfo	ccd;
		ccd.ccdid = ccdidcounter++;
		ccd.name = std::string("external Tracking");
		ccd.size = ImageSize(ccdinforesult.readoutInfo[0].width,
			ccdinforesult.readoutInfo[0].height);
		for (int i = 0; i < ccdinforesult.readoutModes; i++) {
			ccd.binningmodes.insert(SbigMode2Binning(
				ccdinforesult.readoutInfo[i].mode));
		}
		ccdinfo.push_back(ccd);
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera constructor complete");
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

CcdPtr	SbigCamera::getCcd(size_t id) {
	if ((id < 0) || (id >= ccdinfo.size())) {
		throw std::range_error("ccd id not in range");
	}

	CcdInfo	ccd = ccdinfo[id];

	// now that we have he CCD info, we can create a ccd structure
	return CcdPtr(new SbigCcd(ccd, id, *this));
}

FilterWheelPtr	SbigCamera::getFilterWheel() throw (not_implemented) {
	return FilterWheelPtr(new SbigFilterWheel(*this));
}

GuiderPortPtr	SbigCamera::getGuiderPort() throw (not_implemented) {
	return GuiderPortPtr(new SbigGuiderPort(*this));
}

} // namespace sbig
} // namespace camera
} // namespace astro
