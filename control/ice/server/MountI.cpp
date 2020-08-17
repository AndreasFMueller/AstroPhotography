/*
 * MountI.cpp -- 
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <MountI.h>
#include <IceConversions.h>

namespace snowstar {

MountI::MountI(astro::device::MountPtr mount) : DeviceI(*mount), _mount(mount) {
	// register the callback
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%p creating a callback", this);
	MountICallback	*mountcallback = new MountICallback(*this);
	mountcallbackptr = MountICallbackPtr(mountcallback);
	// add the callback to the mount
	debug(LOG_DEBUG, DEBUG_LOG, 0, "add callback");
	_mount->addStatechangeCallback(mountcallbackptr);
	_mount->addPositionCallback(mountcallbackptr);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "callbacks installed");
}

MountI::~MountI() {
	_mount->removeStatechangeCallback(mountcallbackptr);
	_mount->removePositionCallback(mountcallbackptr);
}

RaDec	MountI::getRaDec(const Ice::Current& current) {
	CallStatistics::count(current);
	try {
		return convert(_mount->getRaDec());
	} catch (std::exception x) {
		DeviceException	except;
		except.cause = astro::stringprintf(
			"cannot call getRaDec(): %s", x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", except.cause.c_str());
		throw except;
	}
}

AzmAlt	MountI::getAzmAlt(const Ice::Current& current) {
	CallStatistics::count(current);
	try {
		return convert(_mount->getAzmAlt());
	} catch (std::exception x) {
		DeviceException	except;
		except.cause = astro::stringprintf(
			"cannot call getAzmAlt(): %s", x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", except.cause.c_str());
		throw except;
	}
}

LongLat	MountI::getLocation(const Ice::Current& current) {
	CallStatistics::count(current);
	try {
		astro::LongLat	loc = _mount->location();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "got location: %s",
			loc.toString().c_str());
		return convert(loc);
	} catch (std::exception x) {
		DeviceException	except;
		except.cause = astro::stringprintf("cannot call location(): %s",
			x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", except.cause.c_str());
		throw except;
	}
}

locationtype	MountI::getLocationSource(const Ice::Current& current) {
	CallStatistics::count(current);
	try {
		switch (_mount->location_source()) {
		case astro::device::Mount::LOCAL:	return LocationLOCAL;
		case astro::device::Mount::GPS:		return LocationGPS;
		default:
			throw std::logic_error("unknown location source");
		}
	} catch (std::exception x) {
		DeviceException	except;
		except.cause = astro::stringprintf("cannot call location(): %s",
			x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", except.cause.c_str());
		throw except;
	}
	throw std::logic_error("unreachable state");
}

Ice::Long	MountI::getTime(const Ice::Current& current) {
	CallStatistics::count(current);
	try {
		return _mount->time();
	} catch (std::exception x) {
		DeviceException	except;
		except.cause = astro::stringprintf("cannot call time(): %s",
			x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", except.cause.c_str());
		throw except;
	}
}

void	MountI::cancel(const Ice::Current& current) {
	CallStatistics::count(current);
	try {
		_mount->cancel();
	} catch (std::exception x) {
		DeviceException	except;
		except.cause = astro::stringprintf("cannot call cancel(): %s",
			x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", except.cause.c_str());
		throw except;
	}
}

bool	MountI::telescopePositionWest(const Ice::Current& current) {
	CallStatistics::count(current);
	try {
		return _mount->telescopePositionWest();
	} catch (std::exception x) {
		DeviceException	except;
		except.cause = astro::stringprintf(
			"cannot call telescopePositionWest(): %s",
			x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", except.cause.c_str());
		throw except;
	}
}

void	MountI::GotoAzmAlt(const AzmAlt& azmalt,
		const Ice::Current& current) {
	CallStatistics::count(current);
	try {
		_mount->Goto(convert(azmalt));
	} catch (std::exception x) {
		DeviceException	except;
		except.cause = astro::stringprintf(
			"cannot call Goto(AzmAlt): %s", x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", except.cause.c_str());
		throw except;
	}
}

void	MountI::GotoRaDec(const RaDec& radec,
		const Ice::Current& current) {
	CallStatistics::count(current);
	try {
		_mount->Goto(convert(radec));
	} catch (std::exception x) {
		DeviceException	except;
		except.cause = astro::stringprintf(
			"cannot call Goto(RaDec): %s", x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", except.cause.c_str());
		throw except;
	}
}

mountstate	MountI::state(const Ice::Current& current) {
	CallStatistics::count(current);
	try {
		astro::device::Mount::state_type	s = _mount->state();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "got state %d", s);
		return convert(s);
	} catch (std::exception x) {
		DeviceException	except;
		except.cause = astro::stringprintf(
			"cannot call state(): %s", x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", except.cause.c_str());
		throw except;
	}
}

bool	MountI::hasGuideRates(const Ice::Current& current) {
	CallStatistics::count(current);
	try {
		return _mount->hasGuideRates();
	} catch (std::exception x) {
		DeviceException	except;
		except.cause = astro::stringprintf(
			"cannot call hasGuideRates(): %s", x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", except.cause.c_str());
		throw except;
	}
}

RaDec	MountI::getGuideRates(const Ice::Current& current) {
	CallStatistics::count(current);
	try {
		return convert(_mount->getGuideRates());
	} catch (std::exception x) {
		DeviceException	except;
		except.cause = astro::stringprintf(
			"cannot call getGuideRates(): %s", x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", except.cause.c_str());
		throw except;
	}
}

void	MountI::registerCallback(const Ice::Identity& mountcallback,
		const Ice::Current& current) {
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%p registser callback", this);
	try {
		callbacks.registerCallback(mountcallback, current);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot register callback: %s %s",
			astro::demangle_cstr(x), x.what());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"cannot register callback, unknown reason");
	}
}

void	MountI::unregisterCallback(const Ice::Identity& mountcallback,
		const Ice::Current& current) {
	CallStatistics::count(current);
	try {
		callbacks.unregisterCallback(mountcallback, current);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot unregister callback: %s %s",
			astro::demangle_cstr(x), x.what());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"cannot unregister callback, unknown reason");
	}
}

void	MountI::callbackUpdate(const astro::callback::CallbackDataPtr data) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%p MountI::callbackUpdate called", this);
	try {
		callbacks(data);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot send callback: %s %s",
			astro::demangle_cstr(x), x.what());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot send callback, unknown reason");
	}
}

/**
 * \brief Specialization of the callback_adapter for the MountPrx
 */
template<>
void	callback_adapter<MountCallbackPrx>(MountCallbackPrx& p,
		const astro::callback::CallbackDataPtr data) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "callback_adapter<MountCallbackPrx> called");
	// find out what kinde of data is contained in the envelope
	astro::device::Mount::StateCallbackData	*scd
		= dynamic_cast<astro::device::Mount::StateCallbackData*>(&*data);
	if (scd != NULL) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "state change received");
		p->statechange(convert(scd->data()));
		return;
	}

	astro::device::Mount::PositionCallbackData *pcd
		= dynamic_cast<astro::device::Mount::PositionCallbackData*>(&*data);
	if (pcd != NULL) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "position callback received");
		p->position(convert(pcd->data()));
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "unknown data in callback");
}

} // namespace snowstar
