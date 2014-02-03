/*
 * CoolerI.cpp -- ICE cooler wraper implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <CoolerI.h>

namespace snowstar {

CoolerI::CoolerI(astro::camera::CoolerPtr cooler) : _cooler(cooler) {
}

CoolerI::~CoolerI() {
}

float	CoolerI::getSetTemperature(const Ice::Current& current) {
	return _cooler->getSetTemperature();
}

float	CoolerI::getActualTemperature(const Ice::Current& current) {
	return _cooler->getActualTemperature();
}

void	CoolerI::setTemperature(float temperature, const Ice::Current& current) {
	_cooler->setTemperature(temperature);
}

bool	CoolerI::isOn(const Ice::Current& current) {
	return _cooler->isOn();
}

void	CoolerI::setOn(bool onoff, const Ice::Current& current) {
	_cooler->setOn(onoff);
}

std::string	CoolerI::getName(const Ice::Current& current) {
	return _cooler->name();
}

} // namespace snowstar

