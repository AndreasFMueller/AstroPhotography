/*
 * GuiderPort_impl.cpp -- Corba GuiderPort implementation wrapper
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "GuiderPort_impl.h"
#include <Conversions.h>

namespace Astro {

void	GuiderPort_impl::activate(::CORBA::Float ra, ::CORBA::Float dec) {
	double	raplus = 0, raminus = 0, decplus = 0, decminus = 0;
	if (ra > 0) {
		raplus = ra;
	} else {
		raminus = -ra;
	}
	if (dec > 0) {
		decplus = dec;
	} else {
		decminus = -dec;
	}
	_guiderport->activate(raplus, raminus, decplus, decminus);
}

CORBA::Octet	GuiderPort_impl::active() {
	return convert_relaybits2octet(_guiderport->active());
}

} // namespace Astro
