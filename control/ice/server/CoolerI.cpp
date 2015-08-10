/*
 * CoolerI.cpp -- ICE cooler wraper implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <CoolerI.h>
#include <NameConverter.h>
#include <ProxyCreator.h>

namespace snowstar {

CoolerI::CoolerI(astro::camera::CoolerPtr cooler)
	: DeviceI(*cooler), _cooler(cooler) {
}

CoolerI::~CoolerI() {
}

float	CoolerI::getSetTemperature(const Ice::Current& /* current */) {
	return _cooler->getSetTemperature();
}

float	CoolerI::getActualTemperature(const Ice::Current& /* current */) {
	return _cooler->getActualTemperature();
}

void	CoolerI::setTemperature(float temperature, const Ice::Current& /* current */) {
	_cooler->setTemperature(temperature);
}

bool	CoolerI::isOn(const Ice::Current& /* current */) {
	return _cooler->isOn();
}

void	CoolerI::setOn(bool onoff, const Ice::Current& /* current */) {
	_cooler->setOn(onoff);
}

CoolerPrx	CoolerI::createProxy(const std::string& coolername,
			const Ice::Current& current) {
	return snowstar::createProxy<CoolerPrx>(
		NameConverter::urlencode(coolername), current);
}

} // namespace snowstar

