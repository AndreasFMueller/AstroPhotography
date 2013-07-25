/*
 * SbigGuiderPort.cpp -- SBIG guider port implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SbigLocator.h>
#include <SbigGuiderPort.h>
#include <utils.h>
#include <AstroDebug.h>
#include <sbigudrv.h>

namespace astro {
namespace camera {
namespace sbig {

/**
 * \brief Create a Guiderport object
 *
 * This method assumes that every SBIG camera has a guider port.
 */
SbigGuiderPort::SbigGuiderPort(SbigCamera& _camera) : camera(_camera) {
}

SbigGuiderPort::~SbigGuiderPort() {
}

/**
 * \brief Query the state of the guider port.
 *
 * \returns a bit mask indicating the logical state of the four guider port
 * output relays.
 */
uint8_t	SbigGuiderPort::active() {
	SbigLock	lock;
	camera.sethandle();
	QueryCommandStatusParams	params;
	QueryCommandStatusResults	results;
	params.command = CC_ACTIVATE_RELAY;
	short	e = SBIGUnivDrvCommand(CC_QUERY_COMMAND_STATUS,
		&params, &results);
	if (e != CE_NO_ERROR) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot activate relays: %s",
			sbig_error(e).c_str());
		throw SbigError(e);
	}
	uint8_t	result = 0;
	if (results.status & 0x8) {
		result |= RAPLUS;
	}
	if (results.status & 0x4) {
		result |= RAMINUS;
	}
	if (results.status & 0x2) {
		result |= DECPLUS;
	}
	if (results.status & 0x1) {
		result |= DECMINUS;
	}
	return result;
}

/**
 * \brief Control guider port relays.
 *
 * The parameters indicate which ports have to be enabled for how long
 * (in seconds). 
 * \param raplus	how long to turn on the RA+ output
 * \param raminus	how long to turn on the RA- output
 * \param decplus	how long to turn on the DEC+ output
 * \param decminus	how long to turn on the DEC- output
 */
void	SbigGuiderPort::activate(float raplus, float raminus,
	float decplus, float decminus) {
	SbigLock	lock;
	camera.sethandle();
	ActivateRelayParams	params;
	params.tXPlus = raplus * 100;
	params.tXMinus = raminus * 100;
	params.tYPlus = decplus * 100;
	params.tYMinus = decminus * 100;
	short	e = SBIGUnivDrvCommand(CC_ACTIVATE_RELAY, &params, NULL);
	if (e != CE_NO_ERROR) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot activate relays: %s",
			sbig_error(e).c_str());
		throw SbigError(e);
	}
}

} // namespace sbig
} // namespace camera
} // namespace astro
