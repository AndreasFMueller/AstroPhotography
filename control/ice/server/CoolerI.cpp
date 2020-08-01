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

/**
 * \brief Construct a cooler servant
 *
 * \param cooler	the cooler to encapsulate in the servant
 */
CoolerI::CoolerI(astro::camera::CoolerPtr cooler)
	: DeviceI(*cooler), _cooler(cooler) {
        debug(LOG_DEBUG, DEBUG_LOG, 0, "create a callback");
        CoolerICallback		*coolercallback = new CoolerICallback(*this);
        CoolerICallbackPtr	coolercallbackptr(coolercallback);
        debug(LOG_DEBUG, DEBUG_LOG, 0, "install callback in cooler");
        _cooler->addCallback(coolercallbackptr);
}

/**
 * \brief Destroy the cooler
 */
CoolerI::~CoolerI() {
}

/**
 * \brief Get the set temperature
 *
 * \param current	the current call context
 */
float	CoolerI::getSetTemperature(const Ice::Current& current) {
	CallStatistics::count(current);
	return _cooler->getSetTemperature().temperature();
}

/**
 * \brief Get the actual temperature
 *
 * \param current	the current call context
 */
float	CoolerI::getActualTemperature(const Ice::Current& current) {
	CallStatistics::count(current);
	return _cooler->getActualTemperature().temperature();
}

/**
 * \brief Set the temperature
 *
 * \param temperature	the temperature to set
 * \param current	the current call context
 */
void	CoolerI::setTemperature(float temperature, const Ice::Current& current) {
	CallStatistics::count(current);
	_cooler->setTemperature(astro::Temperature(temperature));
}

/**
 * \brief Whether the cooler is currentl on
 *
 * \param current	the current call context
 */
bool	CoolerI::isOn(const Ice::Current& current) {
	CallStatistics::count(current);
	return _cooler->isOn();
}

/**
 * \brief Switch the cooler on or off
 *
 * \param onoff		the new state of the cooler
 * \param current	the current call context
 */
void	CoolerI::setOn(bool onoff, const Ice::Current& current) {
	CallStatistics::count(current);
	_cooler->setOn(onoff);
}

/**
 * \brief Get the proxy to the cooler
 *
 * \param coolername	the name of the cooler
 * \param current	the current call context
 */
CoolerPrx	CoolerI::createProxy(const std::string& coolername,
			const Ice::Current& current) {
	return snowstar::createProxy<CoolerPrx>(
		NameConverter::urlencode(coolername), current);
}

/**
 * \brief Whether or not the cooler has a dew heater
 *
 * \param current	the current call context
 */
bool	CoolerI::hasDewHeater(const Ice::Current& current) {
	CallStatistics::count(current);
	return _cooler->hasDewHeater();
}

/**
 * \brief Get the current dew heater power value
 *
 * \param current	the current call context
 */
float	CoolerI::getDewHeater(const Ice::Current& current) {
	CallStatistics::count(current);
	return _cooler->dewHeater();
}

/**
 * \brief Set the dew heater power value
 *
 * \param dewheatervalue	the new dew heater power value
 * \param current		the current call context
 */
void	CoolerI::setDewHeater(float dewheatervalue,
		const Ice::Current& current) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new dewheater value: %f", 
		dewheatervalue);
	CallStatistics::count(current);
	_cooler->dewHeater(dewheatervalue);
}

/**
 * \brief Get the range of valid dew heater values
 *
 * \param current	the current call context
 */
Interval	CoolerI::dewHeaterRange(const Ice::Current& current) {
	CallStatistics::count(current);
	std::pair<float, float>	i = _cooler->dewHeaterRange();
	Interval	result;
	result.min = i.first;
	result.max = i.second;
	return result;
}

/**
 * \brief Register a callback
 *
 * \param callback	the callback identity to register
 * \param current	the current call context
 */
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

/**
 * \brief Unregister a callback
 *
 * \param callback	the callback identity to unregister
 * \param current	the current call context
 */
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

/**
 * \brief Update the state 
 *
 * \param data		the callback data to sent to all installed callbacks
 */
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

/**
 * \brief The callback adapter for cooler state updates
 *
 * \param p		the cooler proxy to update
 * \param data		the callback data
 */
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

