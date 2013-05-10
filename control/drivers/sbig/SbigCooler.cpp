/*
 * SbigCooler.cpp -- SBIG cooler implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SbigCooler.h>
#include <sbigudrv.h>
#include <utils.h>
#include <debug.h>

namespace astro {
namespace camera {
namespace sbig {

SbigCooler::SbigCooler(SbigCamera& _camera) : camera(_camera) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "constructing cooler");
	enabled = isOn();
	getSetTemperature();
}

SbigCooler::~SbigCooler() {
}

float	SbigCooler::getSetTemperature() {
	camera.sethandle();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve set temperature");
	QueryTemperatureStatusParams	params;
	QueryTemperatureStatusResults2	results;
	params.request = TEMP_STATUS_ADVANCED2;
	short	e = SBIGUnivDrvCommand(CC_QUERY_TEMPERATURE_STATUS,
		&params, &results);
	if (e != CE_NO_ERROR) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot get set temperature : %s",
			sbig_error(e).c_str());
		throw SbigError(e);
	}
	return temperature = results.ccdSetpoint + 273.1;
}

float	SbigCooler::getActualTemperature() {
	camera.sethandle();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get actual temperature");
	QueryTemperatureStatusParams	params;
	QueryTemperatureStatusResults2	results;
	params.request = TEMP_STATUS_ADVANCED2;
	short	e = SBIGUnivDrvCommand(CC_QUERY_TEMPERATURE_STATUS,
		&params, &results);
	if (e != CE_NO_ERROR) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot get temperature : %s",
			sbig_error(e).c_str());
		throw SbigError(e);
	}
	return results.imagingCCDTemperature + 273.1;
}

void	SbigCooler::setTemperature(const float temperature) {
	camera.sethandle();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set the set temperature");
	this->temperature = temperature;
	if (!isOn()) {
		return;
	}
	set();
}

void	SbigCooler::set() {
	camera.sethandle();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set parameters");
	SetTemperatureRegulationParams2	params;
	params.regulation = (enabled) ? REGULATION_ON : REGULATION_OFF;
	params.ccdSetpoint = temperature - 273.1;
	short	e = SBIGUnivDrvCommand(CC_SET_TEMPERATURE_REGULATION2,
		&params, NULL);
	if (e != CE_NO_ERROR) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot control temperature "
			"regulation: %s", sbig_error(e).c_str());
		throw SbigError(e);
	}
}

bool	SbigCooler::isOn() {
	camera.sethandle();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "query regulation status");
	QueryTemperatureStatusParams	params;
	QueryTemperatureStatusResults	results;
	params.request = TEMP_STATUS_STANDARD;
	short	e = SBIGUnivDrvCommand(CC_QUERY_TEMPERATURE_STATUS,
		&params, &results);
	if (e != CE_NO_ERROR) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot get temperature "
			"status: %s", sbig_error(e).c_str());
		throw SbigError(e);
	}
	return (results.enabled) ? true : false;
}

void	SbigCooler::setOn(bool onoff) {
	enabled = onoff;
	set();
}

} // namespace sbig
} // namespace camera
} // namespace astro
