/*
 * MountI.cpp -- 
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <MountI.h>
#include <IceConversions.h>

namespace snowstar {

MountI::MountI(astro::device::MountPtr mount) : DeviceI(*mount), _mount(mount) {
}

MountI::~MountI() {
}

RaDec	MountI::getRaDec(const Ice::Current& /* current */) {
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

AzmAlt	MountI::getAzmAlt(const Ice::Current& /* current */) {
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

LongLat	MountI::getLocation(const Ice::Current& /* current */) {
	try {
		return convert(_mount->location());
	} catch (std::exception x) {
		DeviceException	except;
		except.cause = astro::stringprintf("cannot call location(): %s",
			x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", except.cause.c_str());
		throw except;
	}
}

Ice::Long	MountI::getTime(const Ice::Current& /* current */) {
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

void	MountI::cancel(const Ice::Current& /* current */) {
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

bool	MountI::telescopePositionWest(const Ice::Current& /* current */) {
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
		const Ice::Current& /* current */) {
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
		const Ice::Current& /* current */) {
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

mountstate	MountI::state(const Ice::Current& /* current */) {
	try {
		return convert(_mount->state());
	} catch (std::exception x) {
		DeviceException	except;
		except.cause = astro::stringprintf(
			"cannot call state(): %s", x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", except.cause.c_str());
		throw except;
	}
}

} // namespace snowstar
