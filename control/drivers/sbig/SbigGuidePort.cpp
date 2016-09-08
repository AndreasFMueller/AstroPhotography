/*
 * SbigGuidePort.cpp -- SBIG guider port implementation
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

#include <SbigLocator.h>
#include <SbigGuidePort.h>
#include <utils.h>
#include <AstroDebug.h>

namespace astro {
namespace camera {
namespace sbig {

/**
 * \brief Create a Guideport object
 *
 * This method assumes that every SBIG camera has a guider port.
 */
SbigGuidePort::SbigGuidePort(SbigCamera& _camera)
	: GuidePort(GuidePort::defaultname(name(), "guideport")),
	  camera(_camera) {
}

SbigGuidePort::~SbigGuidePort() {
}

/**
 * \brief Query the state of the guider port.
 *
 * \returns a bit mask indicating the logical state of the four guider port
 * output relays.
 */
uint8_t	SbigGuidePort::active() {
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
void	SbigGuidePort::activate(float raplus, float raminus,
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
