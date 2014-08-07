/*
 * QsiCooler.cpp -- qsi cooler implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <QsiCooler.h>

namespace astro {
namespace camera {
namespace qsi {

QsiCooler::QsiCooler(QsiCamera& camera)
	: Cooler(DeviceName(camera.name(), DeviceName::Cooler, "cooler")),
	  _camera(camera) {
}

float	QsiCooler::getSetTemperature() {
	double	temp;
	_camera.camera().get_SetCCDTemperature(&temp);
	return temp + 273.13;
}

float	QsiCooler::getActualTemperature() {
	double	temp;
	_camera.camera().get_CCDTemperature(&temp);
	return temp + 273.13;
}

void	QsiCooler::setTemperature(const float temperature) {
	double	temp = temperature - 273.13;
	_camera.camera().put_SetCCDTemperature(temp);
}

bool	QsiCooler::isOn() {
	bool	cooleron;
	_camera.camera().get_CoolerOn(&cooleron);
	return cooleron;
}

void	QsiCooler::setOn(bool onoff) {
	_camera.camera().put_CoolerOn(onoff);
}

} // namespace qsi
} // namespace camera
} // namespace astro
