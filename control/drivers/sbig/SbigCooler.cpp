/*
 * SbigCooler.cpp -- SBIG cooler implementation
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
#include <SbigCooler.h>
#include <utils.h>
#include <AstroDebug.h>

namespace astro {
namespace camera {
namespace sbig {

/**
 * \brief Create an SBIG cooler
 *
 * This is essentiall a holder for cooler specific state and a reference
 * to the camera. The camera contains all the information needed to perform
 * a call to the SBIG universal driver library.
 */
SbigCooler::SbigCooler(SbigCamera& _camera, const DeviceName& name)
	: Cooler(name), SbigDevice(_camera) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "constructing cooler");
	enabled = isOn();
	getSetTemperature();
}

SbigCooler::~SbigCooler() {
}

void	SbigCooler::query_temperature_status(
		QueryTemperatureStatusResults2 *results) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "result structure at %p", results);
	SbigLock	lock;
	camera.sethandle();
	QueryTemperatureStatusParams	params;
	params.request = TEMP_STATUS_ADVANCED2;
	short	e = SBIGUnivDrvCommand(CC_QUERY_TEMPERATURE_STATUS,
			&params, results);
	if (e != CE_NO_ERROR) {
		debug(LOG_ERR, DEBUG_LOG, 0, "can't get temperature status: %s",
			sbig_error(e).c_str());
		throw SbigError(e);
	}
}

/**
 * \brief Query the set temperature
 */
float	SbigCooler::getSetTemperature() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve set temperature from %hd",
		camera.handle);

	QueryTemperatureStatusResults2	results;

	query_temperature_status(&results);

	return temperature = results.ccdSetpoint + 273.1;
}

/**
 * \brief Query the actual temperature
 */
float	SbigCooler::getActualTemperature() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get actual temperature");

	QueryTemperatureStatusResults2	results;

	query_temperature_status(&results);

	return results.imagingCCDTemperature + 273.1;
}

/**
 * \brief Set the set temperature
 */
void	SbigCooler::setTemperature(const float temperature) {
	SbigLock	lock;
	camera.sethandle();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set the set temperature");
	this->temperature = temperature;
	if (!isOn()) {
		return;
	}
	set();
}

void	SbigCooler::set_temperature_regulation2(
		SetTemperatureRegulationParams2 *params) {
	SbigLock	lock;
	camera.sethandle();
	short	e = SBIGUnivDrvCommand(CC_SET_TEMPERATURE_REGULATION2,
		params, NULL);
	if (e != CE_NO_ERROR) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot control temperature "
			"regulation: %s", sbig_error(e).c_str());
		throw SbigError(e);
	}
}

/**
 * \brief Common (private) set function
 */
void	SbigCooler::set() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set parameters");
	SetTemperatureRegulationParams2	params;
	params.regulation = (enabled) ? REGULATION_ON : REGULATION_OFF;
	params.ccdSetpoint = temperature - 273.1;
	set_temperature_regulation2(&params);
}

/**
 * \brief Query whether cooler is on
 */
bool	SbigCooler::isOn() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "query regulation status");
	try {
		QueryTemperatureStatusResults2	results;
		query_temperature_status(&results);
		return (results.coolingEnabled) ? true : false;
	} catch (...) {
		return false;
	}
}

/**
 * \brief Turn cooler on.
 */
void	SbigCooler::setOn(bool onoff) {
	enabled = onoff;
	set();
}

} // namespace sbig
} // namespace camera
} // namespace astro
