/*
 * AdaptiveOpticsI.cpp -- ICE AdaptiveOptics wrapper implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AdaptiveOpticsI.h>
#include <GuidePortI.h>
#include <NameConverter.h>
#include <IceConversions.h>

namespace snowstar {

AdaptiveOpticsI::AdaptiveOpticsI(astro::camera::AdaptiveOpticsPtr ao)
	: DeviceI(*ao), _ao(ao) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create a callback");
	AdaptiveOpticsICallback	*aocallback
		= new AdaptiveOpticsICallback(*this);
	adaptiveopticscallbackptr = AdaptiveOpticsICallbackPtr(aocallback);
	// XXX need to add creation of a callback and installation into
	// XXX the adaptive optics device
	debug(LOG_DEBUG, DEBUG_LOG, 0, "XXX install callback in adaptive optics");
}

AdaptiveOpticsI::~AdaptiveOpticsI() {
	// XXX remove from adaptive optics callbacks
}

void	AdaptiveOpticsI::set(const Point& position, const Ice::Current& current) {
	CallStatistics::count(current);
	_ao->set(convert(position));
}

Point	AdaptiveOpticsI::get(const Ice::Current& current) {
	CallStatistics::count(current);
	return convert(_ao->get());
}

bool	AdaptiveOpticsI::hasGuidePort(const Ice::Current& current) {
	CallStatistics::count(current);
	return _ao->hasGuidePort();
}

GuidePortPrx	AdaptiveOpticsI::getGuidePort(const Ice::Current& current) {
	CallStatistics::count(current);
	std::string	name
		= NameConverter::urlencode(_ao->getGuidePort()->name());
	return GuidePortI::createProxy(name, current);
}

void	AdaptiveOpticsI::center(const Ice::Current& current) {
	CallStatistics::count(current);
	_ao->center();
}

/**
 * \brief Register a callback
 *
 * \param callback	the callback to install
 * \param current	the current call context
 */
void	AdaptiveOpticsI::registerCallback(const Ice::Identity& callback,
		const Ice::Current& current) {
	try {
		callbacks.registerCallback(callback, current);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "can't register callback %s: %s",
			astro::demangle(typeid(x).name()).c_str(), x.what());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"cannot register callback, unknown reason");
	}
}

/**
 * \brief Unregister a callback
 *
 * \param callback	the callback to install
 * \param current	the current call context
 */
void	AdaptiveOpticsI::unregisterCallback(const Ice::Identity& callback,
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
 * \brief Udpate the point
 *
 * \param data 		the callback data to send to the installed callbacks
 */
void	AdaptiveOpticsI::callbackUpdate(const astro::callback::CallbackDataPtr data) {
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
void    callback_adapter<AdaptiveOpticsCallbackPrx>(AdaptiveOpticsCallbackPrx& p,
		const astro::callback::CallbackDataPtr data) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "callback");

	astro::callback::PointCallbackData   *pcd
		= dynamic_cast<astro::callback::PointCallbackData*>(&*data);
	if (NULL != pcd) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "point callback");
		p->point(convert(*pcd));
		return;
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "unknown callback type");
	return;
}

} // namespace snowstar
