/*
 * FilterWheel_impl.cpp -- Corba FilterWheel implementation wrapper
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "FilterWheel_impl.h"

namespace Astro {

CORBA::Long	FilterWheel_impl::nFilters() {
	return _filterwheel->nFilters();
}

CORBA::Long	FilterWheel_impl::currentPosition() {
	return _filterwheel->currentPosition();
}

void	FilterWheel_impl::select(CORBA::Long position) {
	return _filterwheel->select(position);
}

} // namespace Astro
