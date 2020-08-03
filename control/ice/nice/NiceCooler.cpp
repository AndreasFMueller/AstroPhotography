/*
 * NiceCooler.cpp
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <NiceCooler.h>
#include <CommunicatorSingleton.h>
#include <IceConversions.h>

namespace astro {
namespace camera {
namespace nice {

void    NiceCoolerCallbackI::updateCoolerInfo(const snowstar::CoolerInfo& info,
                                const Ice::Current& /* current */) {
	_cooler.callback(convert(info));
}

void    NiceCoolerCallbackI::updateSetTemperature(Ice::Float settemperature,
                                const Ice::Current& /* current */) {
	_cooler.callback(Temperature(settemperature));
}

void    NiceCoolerCallbackI::updateDewHeater(Ice::Float dewheater,
                                const Ice::Current& /* current */) {
	_cooler.callback(DewHeater(dewheater));
}

void    NiceCoolerCallbackI::stop(const Ice::Current& /* current */) {
}


NiceCooler::NiceCooler(snowstar::CoolerPrx cooler, const DeviceName& devicename)
	: Cooler(devicename), NiceDevice(devicename), _cooler(cooler)  {
	_cooler_callback = new NiceCoolerCallbackI(*this);
	_cooler_identity = snowstar::CommunicatorSingleton::add(_cooler_callback);
	_cooler->registerCallback(_cooler_identity);
}

NiceCooler::~NiceCooler() {
	_cooler->unregisterCallback(_cooler_identity);
	snowstar::CommunicatorSingleton::remove(_cooler_identity);
}

Temperature	NiceCooler::getSetTemperature() {
	return Temperature(_cooler->getSetTemperature());
}

Temperature	NiceCooler::getActualTemperature() {
	return Temperature(_cooler->getActualTemperature());
}

void	NiceCooler::setTemperature(float temperature) {
	_cooler->setTemperature(temperature);
}

bool	NiceCooler::isOn() {
	return _cooler->isOn();
}

void	NiceCooler::setOn(bool onoff) {
	_cooler->setOn(onoff);
}

bool	NiceCooler::hasDewHeater() {
	return _cooler->hasDewHeater();
}

float	NiceCooler::dewHeater() {
	return _cooler->getDewHeater();
}

void	NiceCooler::dewHeater(float dewheatervalue) {
	_cooler->setDewHeater(dewheatervalue);
}

std::pair<float, float>	NiceCooler::dewHeaterRange() {
	snowstar::Interval	i = _cooler->dewHeaterRange();
	return std::make_pair((float)i.min, (float)i.max);
}

} // namespace nice
} // namespace camera
} // namespace astro
