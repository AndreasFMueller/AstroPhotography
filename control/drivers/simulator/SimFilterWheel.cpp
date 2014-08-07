/*
 * SimFilterWheel.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SimFilterWheel.h>
#include <AstroExceptions.h>
#include <includes.h>
#include <AstroDebug.h>

namespace astro {
namespace camera {
namespace simulator {

SimFilterWheel::SimFilterWheel(SimLocator& locator)
	: FilterWheel(DeviceName("filterwheel:simulator/filterwheel")),
	  _locator(locator) {
	_currentposition = 0;
	_currentstate = FilterWheel::unknown;
	_changetime = Timer::gettime() + 5;
}

void	SimFilterWheel::checkstate() {
	double	now = Timer::gettime();
	if (now > _changetime) {
		_currentstate = FilterWheel::idle;
		_changetime = now + 1000000;
	}
}

unsigned int	SimFilterWheel::currentPosition() {
	checkstate();
	if (_currentstate != FilterWheel::idle) {
		throw BadState("filterwheel not idle");
	}
	return _currentposition;
}

void    SimFilterWheel::select(size_t filterindex) {
	// make sure the index is legal
	if (filterindex >= 5) {
		throw BadParameter("filterindex may not exceed number "
			"of filters");
	}
	// if we are already at the right position, return
	unsigned int	currentposition = currentPosition();
	if (filterindex == currentposition) {
		return;
	}
	// find out how far we have to move
	int	timedelta = filterindex - currentposition;
	if (timedelta < 0) {
		timedelta = nFilters() + timedelta;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "filterwheel will stop in %d seconds",
		2 * timedelta);
	_changetime = Timer::gettime() + 2 * timedelta;
	_currentstate = FilterWheel::moving;
	_currentposition = filterindex;
}

std::string     SimFilterWheel::filterName(size_t filterindex) {
	switch (filterindex) {
	case 0:	return std::string("L");
	case 1:	return std::string("R");
	case 2:	return std::string("G");
	case 3:	return std::string("B");
	case 4: return std::string("H-alpha");
	}
	throw BadParameter("illegal filter selection");
}

FilterWheel::State	SimFilterWheel::getState() {
	checkstate();
	return _currentstate;
}

} // namespace simulator
} // namespace camera
} // namespace astro
