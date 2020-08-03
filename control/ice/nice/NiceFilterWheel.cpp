/*
 * NiceFilterWheel.cpp -- implementation of wrapper for filter wheels
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <NiceFilterWheel.h>
#include <IceConversions.h>
#include <CommunicatorSingleton.h>

namespace astro {
namespace camera {
namespace nice {

void	NiceFilterWheelCallback::state(snowstar::FilterwheelState s,
                        const Ice::Current& /* current */) {
	_filterwheel.callback(convert(s));
}

void	NiceFilterWheelCallback::position(int filter,
		const Ice::Current& /* current */) {
	_filterwheel.callback(filter);
}

void	NiceFilterWheelCallback::stop(const Ice::Current& /* current */) {
}


NiceFilterWheel::NiceFilterWheel(snowstar::FilterWheelPrx filterwheel,
	const DeviceName& name)
	: FilterWheel(name), NiceDevice(name), _filterwheel(filterwheel) {
	_filterwheel_callback = new NiceFilterWheelCallback(*this);
	_filterwheel_identity = snowstar::CommunicatorSingleton::add(
					_filterwheel_callback);
	_filterwheel->registerCallback(_filterwheel_identity);
}

NiceFilterWheel::~NiceFilterWheel() {
	_filterwheel->unregisterCallback(_filterwheel_identity);
	snowstar::CommunicatorSingleton::remove(_filterwheel_identity);
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
