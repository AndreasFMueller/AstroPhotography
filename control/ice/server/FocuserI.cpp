/*
 * FocuserI.cpp -- ICE Focuser wrapper implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <FocuserI.h>

namespace snowstar {

FocuserI::FocuserI(astro::camera::FocuserPtr focuser) : _focuser(focuser) {
}

FocuserI::~FocuserI() {
}

std::string	FocuserI::getName(const Ice::Current& current) {
	return _focuser->name();
}

int	FocuserI::min(const Ice::Current& current) {
	return _focuser->min();
}

int	FocuserI::max(const Ice::Current& current) {
	return _focuser->max();
}

int	FocuserI::current(const Ice::Current& current) {
	return _focuser->current();
}

int	FocuserI::backlash(const Ice::Current& current) {
	return _focuser->backlash();
}

void	FocuserI::set(int position, const Ice::Current& current) {
	_focuser->set(position);
}

} // namespace snowstar
