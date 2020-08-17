/*
 * FocuserI.cpp -- ICE Focuser wrapper implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <FocuserI.h>
#include "StatisticsI.h"

namespace snowstar {

FocuserI::FocuserI(astro::camera::FocuserPtr focuser)
	: DeviceI(*focuser), _focuser(focuser) {
}

FocuserI::~FocuserI() {
}

int	FocuserI::min(const Ice::Current& current) {
	CallStatistics::count(current);
	return _focuser->min();
}

int	FocuserI::max(const Ice::Current& current) {
	CallStatistics::count(current);
	return _focuser->max();
}

int	FocuserI::current(const Ice::Current& current) {
	CallStatistics::count(current);
	return _focuser->current();
}

int	FocuserI::backlash(const Ice::Current& current) {
	CallStatistics::count(current);
	return _focuser->backlash();
}

void	FocuserI::set(int position, const Ice::Current& current) {
	CallStatistics::count(current);
	_focuser->set(position);
}

/**
 * \brief Register a callback
 *
 * \param callback      the callback identity to register
 * \param current       the current call context
 */
void    FocuserI::registerCallback(const Ice::Identity& callback,
                const Ice::Current& current) {
        try {
                callbacks.registerCallback(callback, current);
        } catch (const std::exception& x) {
                debug(LOG_ERR, DEBUG_LOG, 0, "cannot register callback %s: %s",
                        astro::demangle_cstr(x), x.what());
        } catch (...) {
                debug(LOG_ERR, DEBUG_LOG, 0,
                        "cannot register callback, unknown reason");
        }
}

/**
 * \brief Unregister a callback
 *
 * \param callback      the callback identity to unregister
 * \param current       the current call context
 */
void    FocuserI::unregisterCallback(const Ice::Identity& callback,
                const Ice::Current& current) {
        try {
                callbacks.unregisterCallback(callback, current);
        } catch (const std::exception& x) {
                debug(LOG_ERR, DEBUG_LOG, 0, "can't unregister callback %s: %s",
                        astro::demangle_cstr(x), x.what());
        } catch (...) {
                debug(LOG_ERR, DEBUG_LOG, 0,
                        "cannot register callback, unknown reason");
        }
}

/**
 * \brief Update the state 
 *
 * \param data          the callback data to sent to all installed callbacks
 */
void    FocuserI::callbackUpdate(const astro::callback::CallbackDataPtr data) {
        try {
                callbacks(data);
        } catch (const std::exception& x) {
                debug(LOG_ERR, DEBUG_LOG, 0, "cannot send callback: %s %s",
                astro::demangle_cstr(x), x.what());
        } catch (...) {
                debug(LOG_ERR, DEBUG_LOG, 0,
                        "cannot send callback, unknown reason");
        }
}

/**
 * \brief The callback adapter for cooler state updates
 *
 * \param p             the cooler proxy to update
 * \param data          the callback data
 */
template<>
void    callback_adapter<FocuserCallbackPrx>(FocuserCallbackPrx& /* p */,
                const astro::callback::CallbackDataPtr /* data */) {
        debug(LOG_DEBUG, DEBUG_LOG, 0, "callback");
	return;

        debug(LOG_DEBUG, DEBUG_LOG, 0, "unknown callback type");
        return;
}


} // namespace snowstar
