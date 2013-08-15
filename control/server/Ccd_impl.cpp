/*
 * Ccd_impl.cpp -- Corba Ccd implementation wrapper
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "Ccd_impl.h"

namespace Astro {

bool	Ccd_impl::hasGain() {
	return _ccd->hasGain();
}

bool	Ccd_impl::hasCooler() {
	return _ccd->hasCooler();
}

} // namespace Astro
