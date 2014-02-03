/*
 * SxCooler.cpp -- Starlight Express Cooler API implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SxCcd.h>
#include <AstroUSB.h>
#include <AstroDebug.h>

namespace astro {
namespace camera {
namespace sx {

static DeviceName	sx_coolername(const DeviceName& cameraname) {
	DeviceName	ccdname(cameraname, DeviceName::Ccd, "Imaging");
	DeviceName	coolername(ccdname, DeviceName::Cooler, "cooler");
	return coolername;
}

SxCooler::SxCooler(SxCamera& _camera)
	: Cooler(sx_coolername(_camera.name())), camera(_camera) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create an SX cooler");
	cooler_on = false;
	// we should find out whether the cooler is on, this is done by
	// calling the cooler command twice. The first time we may use the
	// wrong parameters, but when we then retrieve the result, we know
	// the current state, remember it and immediately set it again. So
	// even if our first request was wrong, after the second, we are 
	// back to the original state. We also use the actual temperature
	// for the set temperature, for lack of anything better.
	cmd();
	Cooler::setTemperature(actualtemperature);
	cmd();
}

SxCooler::~SxCooler() {
	// XXX we should turn the cooler off
}

void	SxCooler::cmd() {
	uint16_t	temp = getSetTemperature() * 10;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "cooler command T = %.1f, on = %s",
		getSetTemperature(), (cooler_on) ? "yes" : "no");
	Request<sx_cooler_temperature_t>	request(
		RequestBase::vendor_specific_type,
		RequestBase::device_recipient,
		(uint16_t)((cooler_on) ? 1 : 0),
		(uint8_t)SX_CMD_COOLER, temp);
	camera.controlRequest(&request);
	actualtemperature = request.data()->temperature / 10.;
	cooler_on = (request.data()->status) ? true : false;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "actual temperature = %.1f",
		actualtemperature);
}

float	SxCooler::getActualTemperature() {
	cmd();
	return actualtemperature;
}

void	SxCooler::setTemperature(float temperature) {
	Cooler::setTemperature(temperature);
	cmd();
}

bool	SxCooler::isOn() {
	cmd();
	return cooler_on;
}

void	SxCooler::setOn(bool onoff) {
	cooler_on = onoff;
	cmd();
}

} // namespace sx
} // namespace camera
} // namespace astro
