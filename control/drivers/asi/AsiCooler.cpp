/*
 * AsiCooler.cpp -- implementation of a cooler class for ASI cameras
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AsiCooler.h>

namespace astro {
namespace camera {
namespace asi {

static DeviceName	asiCoolerName(AsiCcd& ccd) {
	return ccd.name().child(DeviceName::Cooler, "cooler");
}

AsiCooler::AsiCooler(AsiCamera& camera, AsiCcd& ccd)
	: Cooler(asiCoolerName(ccd)), _camera(camera) {
}

AsiCooler::~AsiCooler() {
}

float	AsiCooler::getSetTemperature() {
	return 273.1 + _camera.getControlValue(AsiTargetTemp).value / 10.;
}

float	AsiCooler::getActualTemperature() {
	return 273.1 + _camera.getControlValue(AsiTemperature).value / 10.;
}

void	AsiCooler::setTemperature(float temperature) {
	AsiControlValue	value;
	value.type = AsiTargetTemp;
	value.value = 10 * (temperature - 273.1);
	value.isauto = false;
	_camera.setControlValue(value);
}

bool	AsiCooler::isOn() {
	return (_camera.getControlValue(AsiCoolerOn).value) ? true : false;
}

void	AsiCooler::setOn(bool onoff) {
	AsiControlValue	value;
	value.type = AsiCoolerOn;
	value.value = (onoff) ? ASI_TRUE : ASI_FALSE;
	value.isauto = false;
	_camera.setControlValue(value);
}

} // namespace asi
} // namespace camera
} // namespade astro
