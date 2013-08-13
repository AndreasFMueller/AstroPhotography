/*
 * SbigFilterWheel.cpp -- sbig filter wheel implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_SBIGUDRV_H
#include <sbigudrv.h>
#else
#ifdef HAVE_SBIGUDRV_SBIGUDRV_H
#include <SBIGUDrv/sbigudrv.h>
#endif /* HAVE_SBIGUDRV_SBIGUDRV_H */
#endif

#include <SbigLocator.h>
#include <includes.h>
#include <SbigFilterWheel.h>
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <utils.h>
#include <limits>

namespace astro {
namespace camera {
namespace sbig {

/**
 * \brief Initialize the filter wheel
 */
void	SbigFilterWheel::init() {
	// send a CFWC_INIT command to the filter wheel
	CFWParams	params;
	CFWResults	results;
	params.cfwCommand = CFWC_INIT;
	params.cfwModel = CFWSEL_AUTO;
	short	e = SBIGUnivDrvCommand(CC_CFW, &params, &results);
	if (e != CE_NO_ERROR) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot initialize: %s",
		sbig_error(e).c_str());
		throw SbigError(e);
	}

	// wait until the filter wheel settles
	wait();
}

/**
 * \brief Wait until the filter wheel is no longer busy
 */
void	SbigFilterWheel::wait() {
	CFWParams	params;
	CFWResults	results;
	params.cfwCommand = CFWC_QUERY;
	params.cfwModel = CFWSEL_AUTO;
	unsigned int	timeout = 30;
	do {
		short	e = SBIGUnivDrvCommand(CC_CFW, &params, &results);
		if (e != CE_NO_ERROR) {
			debug(LOG_ERR, DEBUG_LOG, 0,
				"cannot open filter wheel: %s",
				sbig_error(e).c_str());
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "status: %s",
			(CFWS_BUSY == results.cfwStatus) ? "BUSY" :
			(CFWS_IDLE == results.cfwStatus) ? "IDLE" : "UNKNOWN");
		if (results.cfwStatus == CFWS_IDLE) {
			if (results.cfwPosition == CFWP_UNKNOWN) {
				debug(LOG_ERR, DEBUG_LOG, 0, "unkown position");
				throw SbigError("unknown position");
			}
			currentindex = results.cfwPosition - 1;
			debug(LOG_DEBUG, DEBUG_LOG, 0, "got position: %u",
				currentindex);
			return;
		}
		sleep(1);
		timeout--;
	} while (timeout > 0);
	debug(LOG_ERR, DEBUG_LOG, 0, "filter wheel goto did not settle");
	throw SbigFilterWheelTimeout("filterwheel timeout");
}

/**
 * \brief construct and SBIG filter wheel
 * 
 * \param camera	SbigCamera object to use when talking to the
 *			Filter wheel
 */
SbigFilterWheel::SbigFilterWheel(SbigCamera& _camera) : camera(_camera) {
	SbigLock	lock;
	camera.sethandle();

	// find out what type of filter wheel we have
	CFWParams	params;
	CFWResults	results;
	params.cfwCommand = CFWC_OPEN_DEVICE;
	params.cfwModel = CFWSEL_AUTO;
	short	e = SBIGUnivDrvCommand(CC_CFW, &params, &results);
	if (e != CE_NO_ERROR) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot open filter wheel: %s",
		sbig_error(e).c_str());
		throw SbigError(e);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "filter wheel version: %hu, "
		"position: %hu, status %hu",
		results.cfwModel, results.cfwPosition, results.cfwStatus);
	if (results.cfwPosition > 0) {
		currentindex = results.cfwPosition - 1;
	} else {
		currentindex = std::numeric_limits<unsigned int>::max();
	}

	// find information about the firmware 
	params.cfwCommand = CFWC_GET_INFO;
	params.cfwParam1 = CFWG_FIRMWARE_VERSION;
	e = SBIGUnivDrvCommand(CC_CFW, &params, &results);
	if (e != CE_NO_ERROR) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot get filter info: %s",
		sbig_error(e).c_str());
		throw SbigError(e);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "filter wheel firmware version: %lu",
		results.cfwResult1);
	npositions = results.cfwResult2;

	// get the current position, doing a reset if necessary
	try {
		wait();
	} catch (SbigFilterWheelTimeout& timeout) {
		debug(LOG_ERR, DEBUG_LOG, 0, "filter wheel timeout, init");
		init();
		wait();
	}

	// report the current position of the filter wheel
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"filter wheel currently in position: %u", currentindex);
}

SbigFilterWheel::~SbigFilterWheel() {
	SbigLock	lock;
	camera.sethandle();
	// find out what type of filter wheel we have
	CFWParams	params;
	CFWResults	results;
	params.cfwCommand = CFWC_CLOSE_DEVICE;
	params.cfwModel = CFWSEL_AUTO;
	short	e = SBIGUnivDrvCommand(CC_CFW, &params, &results);
	if (e != CE_NO_ERROR) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot open filter wheel: %s",
		sbig_error(e).c_str());
		throw SbigError(e);
	}
}

unsigned int	SbigFilterWheel::nFilters() {
	return npositions;
}

/**
 * \brief Determine current filter wheel position.
 */
unsigned int	SbigFilterWheel::currentPosition() {
	SbigLock	lock;
	camera.sethandle();
	wait();
	return currentindex;
}

/**
 * \brief Move the filter wheel to a given position
 *
 * \param filterindex	index of the filter position.
 */
void	SbigFilterWheel::select(size_t filterindex) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "filterindex %u requested", filterindex);
#if 1
	if (currentindex == filterindex) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"we are already in position %d", filterindex);
		return;
	}
#endif
	SbigLock	lock;
	camera.sethandle();
	CFWParams	params;
	CFWResults	results;
	params.cfwCommand = CFWC_GOTO;
	params.cfwModel = CFWSEL_AUTO;
	params.cfwParam1 = filterindex + 1;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "positioning on %hu", params.cfwParam1);
	short	e = SBIGUnivDrvCommand(CC_CFW, &params, &results);
	if (e != CE_NO_ERROR) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot position filter wheel: %s",
			sbig_error(e).c_str());
	}

	// now query the position, this will block until the filter wheel
	// is positioned
	if (filterindex != currentPosition()) {
		throw SbigError("position mismatch");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "filter wheel positioned");
}

/**
 * \brief find the name of the filter wheel position
 */
std::string	SbigFilterWheel::filterName(size_t filterindex) {
	return stringprintf("filter position %u", filterindex);
}

} // namespace sbig
} // namespace camera
} // namespace astro

