/*
 * NiceFilterWheel.cpp -- implementation of wrapper for filter wheels
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <NiceFilterWheel.h>
#include <IceConversions.h>

namespace astro {
namespace camera {
namespace nice {

NiceFilterWheel::NiceFilterWheel(snowstar::FilterWheelPrx filterwheel,
	const DeviceName& name)
	: FilterWheel(name), NiceDevice(name), _filterwheel(filterwheel) {
}

NiceFilterWheel::~NiceFilterWheel() {
}

unsigned int	NiceFilterWheel::nFilters() {
	return _filterwheel->nFilters();
}

unsigned int	NiceFilterWheel::currentPosition() {
	return _filterwheel->currentPosition();
}

void	NiceFilterWheel::select(size_t filterindex) {
	_filterwheel->select(filterindex);
}

void	NiceFilterWheel::select(const std::string& name) {
	_filterwheel->selectName(name);
}

std::string	NiceFilterWheel::filterName(size_t filterindex) {
	return _filterwheel->filterName(filterindex);
}

FilterWheel::State	NiceFilterWheel::getState() {
	return convert(_filterwheel->getState());
}

} // namespace nice
} // namespace camera
} // namespace astro
