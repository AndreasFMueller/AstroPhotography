/*
 * FilterWheel_impl.cpp -- Corba FilterWheel implementation wrapper
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "FilterWheel_impl.h"

namespace Astro {

/**
 * \brief Get the number of Filters
 */
CORBA::Long	FilterWheel_impl::nFilters() {
	return _filterwheel->nFilters();
}

/**
 * \brief get the current position of the filter wheel
 */
CORBA::Long	FilterWheel_impl::currentPosition() {
	return _filterwheel->currentPosition();
}

/**
 * \brief Select a specific filter
 */
void	FilterWheel_impl::select(CORBA::Long position) {
	if ((position < 0) || (position >= _filterwheel->nFilters())) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"filter wheel position %d out of range", position);
		NotFound	notfound;
		notfound.cause
			= (const char *)"filter wheel position out of range";
		throw notfound;
	}
	return _filterwheel->select(position);
}

/**
 * \brief Retrieve the filter name
 */
char	*FilterWheel_impl::filterName(CORBA::Long position) {
	if ((position < 0) || (position >= _filterwheel->nFilters())) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"filter wheel position %d out of range", position);
		NotFound	notfound;
		notfound.cause
			= (const char *)"filter wheel position out of range";
		throw notfound;
	}
	return CORBA::string_dup(_filterwheel->filterName(position).c_str());
}

} // namespace Astro
