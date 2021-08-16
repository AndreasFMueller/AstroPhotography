/*
 * NiceMount.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <NiceMount.h>
#include <IceConversions.h>
#include <CommunicatorSingleton.h>

namespace astro {
namespace device {
namespace nice {

void	NiceMountCallbackI::statechange(snowstar::mountstate s,
		const Ice::Current& /* current */) {
	_mount.callback(convert(s));
}

void	NiceMountCallbackI::position(const snowstar::RaDec& newposition,
		const Ice::Current& /* current */) {
	_mount.callback(convert(newposition));
}

void	NiceMountCallbackI::stop(const Ice::Current& /* current */) {
}

NiceMount::NiceMount(snowstar::MountPrx mount, const DeviceName& devicename)
	: Mount(devicename), _mount(mount) {
	_mount_callback = new NiceMountCallbackI(*this);
	_mount_identity = snowstar::CommunicatorSingleton::add(_mount_callback);
	_mount->registerCallback(_mount_identity);
}

NiceMount::~NiceMount() {
	_mount->unregisterCallback(_mount_identity);
	snowstar::CommunicatorSingleton::remove(_mount_identity);
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

Mount::location_source_type	NiceMount::location_source() {
	switch (_mount->getLocationSource()) {
	case snowstar::LocationLOCAL:	return Mount::LOCAL;
	case snowstar::LocationGPS:	return Mount::GPS;
	default:
		throw std::logic_error("unknown location source");
	}
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

bool    NiceMount::trackingNorth() {
	return _mount->trackingNorth();
}

void    NiceMount::cancel() {
	_mount->cancel();
}

} // namespace nice
} // namespace device
} // namespace astro
