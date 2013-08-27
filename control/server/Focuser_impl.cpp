/*
 * Focuser_impl.cpp -- Focuser servant implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Focuser_impl.h>

namespace Astro {

CORBA::UShort	Focuser_impl::min() {
	return _focuser->min();
}

CORBA::UShort	Focuser_impl::max() {
	return _focuser->max();
}

CORBA::UShort	Focuser_impl::current() {
	return _focuser->current();
}

void	Focuser_impl::set(CORBA::UShort value) {
	_focuser->set(value);
}

} // namespace Astro
