/*
 * SimFilterWheel.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SimFilterWheel.h>
#include <AstroExceptions.h>
#include <includes.h>

namespace astro {
namespace camera {
namespace simulator {

SimFilterWheel::SimFilterWheel(SimLocator& locator)
	: FilterWheel(DeviceName("filterwheel:sx/filterwheel")),
	  _locator(locator) {
	_currentPosition = 0;
}

void    SimFilterWheel::select(size_t filterindex) {
	if (filterindex >= 5) {
		throw BadParameter("filterindex may not exceed number "
			"of filters");
	}
	sleep(3);
	_currentPosition = filterindex;
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

} // namespace simulator
} // namespace camera
} // namespace astro
