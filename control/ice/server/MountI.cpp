/*
 * MountI.cpp -- 
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <MountI.h>
#include <TypesI.h>

namespace snowstar {

mountstate	convert(astro::device::Mount::mount_state s) {
	switch (s) {
	case astro::device::Mount::IDLE:
		return MountIDLE;
	case astro::device::Mount::TRACKING:
		return MountTRACKING;
	case astro::device::Mount::GOTO:
		return MountGOTO;
	}
	throw std::runtime_error("unknown state");
}

astro::device::Mount::mount_state	convert(mountstate s) {
	switch (s) {
	case MountIDLE:
		return astro::device::Mount::IDLE;
	case MountTRACKING:
		return astro::device::Mount::TRACKING;
	case MountGOTO:
		return astro::device::Mount::GOTO;
	}
	throw std::runtime_error("unknown state");
}

MountI::MountI(astro::device::MountPtr mount) : _mount(mount) {
}

MountI::~MountI() {
}

RaDec	MountI::getRaDec(const Ice::Current& /* current */) {
	return convert(_mount->getRaDec());
}

AzmAlt	MountI::getAzmAlt(const Ice::Current& /* current */) {
	return convert(_mount->getAzmAlt());
}

void	MountI::cancel(const Ice::Current& /* current */) {
	_mount->cancel();
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
