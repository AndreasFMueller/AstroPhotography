/*
 * SbigFilterWheel.cpp -- sbig filter wheel implementation
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
#include <includes.h>
#include <SbigFilterWheel.h>
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <utils.h>
#include <limits>

namespace astro {
namespace camera {
namespace sbig {

void	SbigFilterWheel::cfw(CFWParams *params, CFWResults *results,
		const std::string& msg) {
	SbigLock	lock;
	camera.sethandle();
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"CFW command, model=%d, command=%d, param1=%lu, param2=%lu",
		params->cfwModel, params->cfwCommand,
		params->cfwParam1, params->cfwParam2);
	short	e = SBIGUnivDrvCommand(CC_CFW, &params, &results);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "error code=%hd", e);
	if (e != CE_NO_ERROR) {
		debug(LOG_ERR, DEBUG_LOG, 0, "%s: %s", msg.c_str(),
			sbig_error(e).c_str());
		throw SbigError(e);
	}

}

/**
 * \brief Initialize the filter wheel
 */
void	SbigFilterWheel::init() {
	// send a CFWC_INIT command to the filter wheel
	CFWParams	params;
	CFWResults	results;
	params.cfwCommand = CFWC_INIT;
	params.cfwModel = CFWSEL_AUTO;
	cfw(&params, &results, "cannot initialize");
#if 0
	short	e = SBIGUnivDrvCommand(CC_CFW, &params, &results);
	if (e != CE_NO_ERROR) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot initialize: %s",
		sbig_error(e).c_str());
		throw SbigError(e);
	}
#endif

	// wait until the filter wheel settles
	wait();
}

/**
 * \brief Wait until the filter wheel is no longer busy
 */
void	SbigFilterWheel::wait() {
	CFWParams	params;
	memset(&params, 0, sizeof(params));
	CFWResults	results;
	params.cfwCommand = CFWC_QUERY;
	params.cfwModel = CFWSEL_AUTO;
	unsigned int	timeout = 30;
	do {
		try {
			cfw(&params, &results, "cannot open filter wheel");
		} catch (...) {
			// XXX what if
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
SbigFilterWheel::SbigFilterWheel(SbigCamera& _camera)
	: FilterWheel(FilterWheel::defaultname(name(), "filterwheel")),
	  SbigDevice(_camera) {
	SbigLock	lock;
	camera.sethandle();

	// find out what type of filter wheel we have
	CFWParams	params;
	memset(&params, 0, sizeof(params));
	CFWResults	results;
	params.cfwCommand = CFWC_OPEN_DEVICE;
	params.cfwModel = CFWSEL_AUTO;
	params.cfwParam1 = CFWPORT_COM1;
	cfw(&params, &results, "cannot open filter wheel");
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
	cfw(&params, &results, "cannot get filter info");
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
	// find out what type of filter wheel we have
	CFWParams	params;
	CFWResults	results;
	params.cfwCommand = CFWC_CLOSE_DEVICE;
	params.cfwModel = CFWSEL_AUTO;
	cfw(&params, &results, "cannot close filter wheel");
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
	CFWParams	params;
	memset(&params, 0, sizeof(params));
	params.cfwCommand = CFWC_GOTO;
	params.cfwModel = CFWSEL_AUTO;
	CFWResults	results;
	params.cfwParam1 = filterindex + 1;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "positioning on %hu", params.cfwParam1);
	try {
		cfw(&params, &results, "cannot position filter wheel");
	} catch (...) {
		// XXX what if
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

/**
 * \brief find the current filter wheel state
 */
FilterWheel::State	SbigFilterWheel::getState() {
	return state();
}

FilterWheel::State	SbigFilterWheel::state() {
	CFWParams	params;
	memset(&params, 0, sizeof(params));
	params.cfwCommand = CFWC_QUERY;
	params.cfwModel = CFWSEL_AUTO;
	CFWResults	results;
	try {
		cfw(&params, &results, "cannot open filter wheel");
	} catch (...) {
		// XXX what if
	}
	// if the filter wheel is idle, it could still be that we are
	// in the unknown state, because we don't know the position
	if (CFWS_IDLE == results.cfwStatus) {
		if (results.cfwPosition == CFWP_UNKNOWN) {
			return FilterWheel::unknown;
		}
		currentindex = results.cfwPosition - 1;
		return FilterWheel::idle;
	}
	// if the filter wheel is busy, then it is moving
	if (CFWS_BUSY == results.cfwStatus) {
		return FilterWheel::moving;
	}
	// at this position, we really don't know what to do
	throw SbigError("don't know the current state");
}

} // namespace sbig
} // namespace camera
} // namespace astro

