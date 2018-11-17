/*
 * NiceMount.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <NiceMount.h>
#include <IceConversions.h>

namespace astro {
namespace device {
namespace nice {

NiceMount::NiceMount(snowstar::MountPrx mount, const DeviceName& devicename)
	: Mount(devicename), _mount(mount) {
}

NiceMount::~NiceMount() {
}

astro::device::Mount::state_type        NiceMount::state() {
	return convert(_mount->state());
}

RaDec   NiceMount::getRaDec() {
	return convert(_mount->getRaDec());
}

AzmAlt  NiceMount::getAzmAlt() {
	return convert(_mount->getAzmAlt());
}

LongLat NiceMount::location() {
	LongLat	l = convert(_mount->getLocation());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got remote location: %s",
		l.toString().c_str());
	return l;
}

time_t  NiceMount::time() {
	return _mount->getTime();
}

void    NiceMount::Goto(const RaDec& radec) {
	_mount->GotoRaDec(snowstar::convert(radec));
}

void    NiceMount::Goto(const AzmAlt& azmalt) {
	return _mount->GotoAzmAlt(snowstar::convert(azmalt));
}

bool    NiceMount::telescopePositionWest() {
	return _mount->telescopePositionWest();
}

void    NiceMount::cancel() {
	_mount->cancel();
}

} // namespace nice
} // namespace device
} // namespace astro
