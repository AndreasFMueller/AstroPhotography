/*
 * QsiCooler.cpp -- qsi cooler implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <QsiCooler.h>

namespace astro {
namespace camera {
namespace qsi {

DeviceName	coolername(const DeviceName& cameraname) {
	return cameraname
			.child(DeviceName::Ccd, "ccd")
			.child(DeviceName::Cooler, "cooler");
}

QsiCooler::QsiCooler(QsiCamera& camera)
	: Cooler(coolername(camera.name())), _camera(camera) {
}

float	QsiCooler::getSetTemperature() {
	std::unique_lock<std::recursive_mutex>	lock(_camera.mutex);
	double	temp;
	_camera.camera().get_SetCCDTemperature(&temp);
	return temp + 273.13;
}

float	QsiCooler::getActualTemperature() {
	std::unique_lock<std::recursive_mutex>	lock(_camera.mutex);
	double	temp;
	_camera.camera().get_CCDTemperature(&temp);
	return temp + 273.13;
}

void	QsiCooler::setTemperature(const float temperature) {
	std::unique_lock<std::recursive_mutex>	lock(_camera.mutex);
	double	temp = temperature - 273.13;
	_camera.camera().put_SetCCDTemperature(temp);
}

bool	QsiCooler::isOn() {
	std::unique_lock<std::recursive_mutex>	lock(_camera.mutex);
	bool	cooleron;
	_camera.camera().get_CoolerOn(&cooleron);
	return cooleron;
}

void	QsiCooler::setOn(bool onoff) {
	std::unique_lock<std::recursive_mutex>	lock(_camera.mutex);
	_camera.camera().put_CoolerOn(onoff);
}

} // namespace qsi
} // namespace camera
} // namespace astro
