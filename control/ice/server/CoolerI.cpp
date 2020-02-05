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

float	CoolerI::getSetTemperature(const Ice::Current& current) {
	CallStatistics::count(current);
	return _cooler->getSetTemperature().temperature();
}

float	CoolerI::getActualTemperature(const Ice::Current& current) {
	CallStatistics::count(current);
	return _cooler->getActualTemperature().temperature();
}

void	CoolerI::setTemperature(float temperature, const Ice::Current& current) {
	CallStatistics::count(current);
	_cooler->setTemperature(astro::Temperature(temperature));
}

bool	CoolerI::isOn(const Ice::Current& current) {
	CallStatistics::count(current);
	return _cooler->isOn();
}

void	CoolerI::setOn(bool onoff, const Ice::Current& current) {
	CallStatistics::count(current);
	_cooler->setOn(onoff);
}

CoolerPrx	CoolerI::createProxy(const std::string& coolername,
			const Ice::Current& current) {
	return snowstar::createProxy<CoolerPrx>(
		NameConverter::urlencode(coolername), current);
}

bool	CoolerI::hasDewHeater(const Ice::Current& current) {
	CallStatistics::count(current);
	return _cooler->hasDewHeater();
}

float	CoolerI::getDewHeater(const Ice::Current& current) {
	CallStatistics::count(current);
	return _cooler->dewHeater();
}

void	CoolerI::setDewHeater(float dewheatervalue,
		const Ice::Current& current) {
	CallStatistics::count(current);
	_cooler->dewHeater(dewheatervalue);
}

Interval	CoolerI::dewHeaterRange(const Ice::Current& current) {
	CallStatistics::count(current);
	std::pair<float, float>	i = _cooler->dewHeaterRange();
	Interval	result;
	result.min = i.first;
	result.max = i.second;
	return result;
}

} // namespace snowstar

