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
	if ((position < 0) || (position >= _filterwheel->nFilters())) {
		debug(LOG_ERR, DEBUG_LOG, 0, "filter wheel position %d out of range", position);
		NotFound	notfound;
		notfound.cause = (const char *)"filter wheel position out of range";
		throw notfound;
	}
	return _filterwheel->select(position);
}

} // namespace Astro
