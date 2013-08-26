/*
 * NetFilterWheel.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswi
 */
#include <NetFilterWheel.h>

namespace astro {
namespace camera {
namespace net {

NetFilterWheel::NetFilterWhell(Astro::FilterWheel_var filterwheel)
	: _filterwheel(filterwheel) {
}

int	NetFilterWheel::nFilters() {
	return filterwheel->nFilters();
}

int	NetFilterWheel::currentPosition() {
	return filterwheel->currentPosition();
}

void	NetFilterWheel::select(size_t filterindex) {
	filterwheel->select(filterindex);
}

} // namespace net
} // namespace camera
} // namespace astro
