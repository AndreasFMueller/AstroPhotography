/*
 * CoolerI.cpp -- ICE cooler wraper implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <CoolerI.h>
#include <NameConverter.h>
#include <ProxyCreator.h>
#include <IceConversions.h>

namespace snowstar {

CoolerI::CoolerI(astro::camera::CoolerPtr cooler)
	: DeviceI(*cooler), _cooler(cooler) {
        debug(LOG_DEBUG, DEBUG_LOG, 0, "create a callback");
        CoolerICallback		*coolercallback = new CoolerICallback(*this);
        CoolerICallbackPtr	coolercallbackptr(coolercallback);
        debug(LOG_DEBUG, DEBUG_LOG, 0, "install callback in cooler");
        _cooler->addCallback(coolercallbackptr);
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new dewheater value: %f", 
		dewheatervalue);
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

void	CoolerI::registerCallback(const Ice::Identity& callback,
		const Ice::Current& current) {
	try {
		callbacks.registerCallback(callback, current);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot register callback %s: %s",
			astro::demangle(typeid(x).name()).c_str(), x.what());
        } catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"cannot register callback, unknown reason");
	}
}

void	CoolerI::unregisterCallback(const Ice::Identity& callback,
		const Ice::Current& current) {
	try {
		callbacks.unregisterCallback(callback, current);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "can't unregister callback %s: %s",
			astro::demangle(typeid(x).name()).c_str(), x.what());
        } catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"cannot register callback, unknown reason");
	}
}

void	CoolerI::callbackUpdate(const astro::callback::CallbackDataPtr data) {
	try {
		callbacks(data);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot send callback: %s %s",
		astro::demangle(typeid(x).name()).c_str(), x.what());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"cannot send callback, unknown reason");
	}
}

template<>
void	callback_adapter<CoolerCallbackPrx>(CoolerCallbackPrx& p,
		const astro::callback::CallbackDataPtr data) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "callback");

	astro::camera::CoolerInfoCallbackData	*ci
		= dynamic_cast<astro::camera::CoolerInfoCallbackData*>(&*data);
	if (NULL != ci) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "info callback");
		p->updateCoolerInfo(convert(*ci));
		return;
	}

	astro::camera::SetTemperatureCallbackData	*temp
		= dynamic_cast<astro::camera::SetTemperatureCallbackData*>(&*data);
	if (NULL != temp) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "temperature callback");
		p->updateSetTemperature(temp->data().temperature());
		return;
	}

	astro::camera::DewHeaterCallbackData	*dewheater
		= dynamic_cast<astro::camera::DewHeaterCallbackData*>(&*data);
	if (NULL != dewheater) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "dewheater callback");
		p->updateDewHeater(dewheater->data().dewheater);
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "unknown callback type");
	return;
}


} // namespace snowstar

