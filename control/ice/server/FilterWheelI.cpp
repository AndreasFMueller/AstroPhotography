/*
 * FilterWheelI.cpp -- ICE FilterWheel implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <FilterWheelI.h>
#include <NameConverter.h>
#include <ProxyCreator.h>
#include <IceConversions.h>

namespace snowstar {

/**
 * \brief Construct a filterwheel servant
 *
 * \param filterwheel		the filterwheel to encapsulate
 */
FilterWheelI::FilterWheelI(astro::camera::FilterWheelPtr filterwheel)
		: DeviceI(*filterwheel), _filterwheel(filterwheel) {
	FilterWheelICallback	*filterwheelcallback
		= new FilterWheelICallback(*this);
	filterwheelcallbackptr = FilterWheelICallbackPtr(filterwheelcallback);
	_filterwheel->addCallback(filterwheelcallbackptr);
}

/**
 * \brief Destroy the Filterwheel servant
 */
FilterWheelI::~FilterWheelI() {
	_filterwheel->removeCallback(filterwheelcallbackptr);
}

/**
 * \brief Find out how many filter the wheel has
 *
 * \param current	the current call context
 */
int	FilterWheelI::nFilters(const Ice::Current& current) {
	CallStatistics::count(current);
	return _filterwheel->nFilters();
}

/**
 * \brief Find the current filter position
 *
 * \param current	the current call context
 */
int	FilterWheelI::currentPosition(const Ice::Current& current) {
	CallStatistics::count(current);
	return _filterwheel->currentPosition();
}

/**
 * \brief select a specific filter
 *
 * \param position	the position to select
 * \param current	the current call context
 */
void	FilterWheelI::select(int position, const Ice::Current& current) {
	CallStatistics::count(current);
	return _filterwheel->select(position);
}

/**
 * \brief Select a filter by name
 *
 * \param filtername	name of the filter to select
 * \param current	the current call context
 */
void	FilterWheelI::selectName(const std::string& filtername,
		const Ice::Current& current) {
	CallStatistics::count(current);
	return _filterwheel->select(filtername);
}

/**
 * \brief Name of the filter in a given position 
 *
 * \param position	the position to name
 * \param current	the current call context
 */
std::string	FilterWheelI::filterName(int position,
			const Ice::Current& current) {
	CallStatistics::count(current);
	return _filterwheel->filterName(position);
}

/**
 * \brief Get the current filterwheel state
 *
 * \param current	the  current call context
 */
FilterwheelState	FilterWheelI::getState(const Ice::Current& current) {
	CallStatistics::count(current);
	return convert(_filterwheel->getState());
}

/**
 * \brief Create a proxy fot eh filterwhile from the name
 *
 * \param filterhweelname	the filterwheel name
 * \param current		the current call context
 */
FilterWheelPrx	FilterWheelI::createProxy(const std::string& filterwheelname,
	const Ice::Current& current) {
	return snowstar::createProxy<FilterWheelPrx>(
		NameConverter::urlencode(filterwheelname), current);
}


/**
 * \brief Register a callback
 *
 * \param callback	the callback identity to register
 * \param current	the current call context
 */
void	FilterWheelI::registerCallback(const Ice::Identity& callback,
		const Ice::Current& current) {
	CallStatistics::count(current);
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
 * \param callback	the callback identity to unregister
 * \param current	the current call context
 */
void	FilterWheelI::unregisterCallback(const Ice::Identity& callback,
		const Ice::Current& current) {
	CallStatistics::count(current);
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
 * \param data		the callback data to sent to all installed callbacks
 */
void	FilterWheelI::callbackUpdate(const astro::callback::CallbackDataPtr data) {
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
 * \param p		the cooler proxy to update
 * \param data		the callback data
 */
template<>
void	callback_adapter<FilterWheelCallbackPrx>(FilterWheelCallbackPrx p,
		const astro::callback::CallbackDataPtr data) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "callback");
	// check for filter state
	astro::camera::FilterWheelStateCallbackData	*fcbd
		= dynamic_cast<astro::camera::FilterWheelStateCallbackData*>(&*data);
	if (NULL != fcbd) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "new filterwheel state %s",
			astro::camera::FilterWheel::state2string(fcbd->data()).c_str());
		try {
			p->state(convert(fcbd->data()));
			return;
		} catch (const std::exception& x) {
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot send state: %s",
				x.what());
			throw x;
		}
	}

	// check for filter position
	astro::callback::IntegerCallbackData	*icd
		= dynamic_cast<astro::callback::IntegerCallbackData*>(&*data);
	if (NULL != icd) {
		int	filter = icd->value();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found integer filter = %d",
			filter);
		try {
			p->position(filter);
			return;
		} catch (const std::exception& x) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"exception in filter callback: %s", x.what());
			throw x;
		}
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "unknown callback type");
	return;
}

} // namespace snowstar

