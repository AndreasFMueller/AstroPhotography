/*
 * Focuser_impl.cpp -- Focuser servant implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Focuser_impl.h>

namespace Astro {

char	*Focuser_impl::getName() {
	std::string	name = _focuser->name();
	return CORBA::string_dup(name.c_str());
}

CORBA::UShort	Focuser_impl::min() {
	return _focuser->min();
}

CORBA::UShort	Focuser_impl::max() {
	return _focuser->max();
}

CORBA::UShort	Focuser_impl::current() {
	return _focuser->current();
}

CORBA::UShort	Focuser_impl::backlash() {
	return _focuser->backlash();
}

void	Focuser_impl::set(CORBA::UShort value) {
	_focuser->set(value);
}

} // namespace Astro
