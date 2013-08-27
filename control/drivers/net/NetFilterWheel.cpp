/*
 * NetFilterWheel.cpp -- network based filter wheel implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswi
 */
#include <NetFilterWheel.h>

namespace astro {
namespace camera {
namespace net {

NetFilterWheel::NetFilterWheel(Astro::FilterWheel_var filterwheel)
	: _filterwheel(filterwheel) {
	Astro::FilterWheel_Helper::duplicate(_filterwheel);
}

NetFilterWheel::~NetFilterWheel() {
	Astro::FilterWheel_Helper::release(_filterwheel);
}

unsigned int	NetFilterWheel::nFilters() {
	return _filterwheel->nFilters();
}

unsigned int	NetFilterWheel::currentPosition() {
	return _filterwheel->currentPosition();
}

void	NetFilterWheel::select(size_t filterindex) {
	_filterwheel->select(filterindex);
}

std::string	NetFilterWheel::filterName(size_t filterindex) {
	return std::string();
}

} // namespace net
} // namespace camera
} // namespace astro
