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
	return convert(_mount->getRaDec());
}

AzmAlt	MountI::getAzmAlt(const Ice::Current& /* current */) {
	return convert(_mount->getAzmAlt());
}

LongLat	MountI::getLocation(const Ice::Current& /* current */) {
	return convert(_mount->location());
}

long	MountI::getTime(const Ice::Current& /* current */) {
	return _mount->time();
}

void	MountI::cancel(const Ice::Current& /* current */) {
	_mount->cancel();
}

bool	MountI::telescopePositionWest(const Ice::Current& /* current */) {
	return _mount->telescopePositionWest();
}

void	MountI::GotoAzmAlt(const AzmAlt& azmalt,
		const Ice::Current& /* current */) {
	_mount->Goto(convert(azmalt));
}

void	MountI::GotoRaDec(const RaDec& radec,
		const Ice::Current& /* current */) {
	_mount->Goto(convert(radec));
}

mountstate	MountI::state(const Ice::Current& /* current */) {
	return convert(_mount->state());
}

} // namespace snowstar
