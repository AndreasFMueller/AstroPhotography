/*
 * NetFilterWheel.cpp -- network based filter wheel implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswi
 */
#include <NetFilterWheel.h>
#include <AstroUtils.h>
#include <NetUtils.h>

namespace astro {
namespace camera {
namespace net {

/**
 * \brief Create a Filter Wheel
 *
 * The constructor keeps a reference the filter wheel
 */
NetFilterWheel::NetFilterWheel(Astro::FilterWheel_var filterwheel)
	: FilterWheel(devname2netname(_filterwheel->getName())),
	  _filterwheel(filterwheel) {
	Astro::FilterWheel_Helper::duplicate(_filterwheel);
}

/**
 * \brief Destroy the Filter Wheel
 *
 * Release the reference to the filter wheel
 */
NetFilterWheel::~NetFilterWheel() {
	Astro::FilterWheel_Helper::release(_filterwheel);
}

/**
 * \brief Get the number of Filters in the filter wheel
 */
unsigned int	NetFilterWheel::nFilters() {
	return _filterwheel->nFilters();
}

/**
 * \brief Get the current position 
 */
unsigned int	NetFilterWheel::currentPosition() {
	return _filterwheel->currentPosition();
}

/**
 * \brief Select one of the filters
 */
void	NetFilterWheel::select(size_t filterindex) {
	_filterwheel->select(filterindex);
}

/**
 * \brief Get the filter name
 */
std::string	NetFilterWheel::filterName(size_t filterindex) {
	return std::string(_filterwheel->filterName(filterindex));
}

} // namespace net
} // namespace camera
} // namespace astro
