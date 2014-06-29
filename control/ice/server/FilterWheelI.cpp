/*
 * FilterWheelI.cpp -- ICE FilterWheel implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <FilterWheelI.h>
#include <NameConverter.h>
#include <ProxyCreator.h>

namespace snowstar {

FilterWheelI::~FilterWheelI() {
}

std::string	FilterWheelI::getName(const Ice::Current& current) {
	return _filterwheel->name().toString();
}

int	FilterWheelI::nFilters(const Ice::Current& current) {
	return _filterwheel->nFilters();
}

int	FilterWheelI::currentPosition(const Ice::Current& current) {
	return _filterwheel->currentPosition();
}

void	FilterWheelI::select(int position, const Ice::Current& current) {
	return _filterwheel->select(position);
}

std::string	FilterWheelI::filterName(int position,
			const Ice::Current& current) {
	return _filterwheel->filterName(position);
}

FilterwheelState	FilterWheelI::convert(const astro::camera::FilterWheel::State& s) {
	switch (s) {
	case astro::camera::FilterWheel::idle:
		return snowstar::FwIDLE;
	case astro::camera::FilterWheel::moving:
		return snowstar::FwMOVING;
	case astro::camera::FilterWheel::unknown:
		return snowstar::FwUNKNOWN;
	}
}

FilterwheelState	FilterWheelI::getState(const Ice::Current& current) {
	return convert(_filterwheel->getState());
}

FilterWheelPrx	FilterWheelI::createProxy(const std::string& filterwheelname,
	const Ice::Current& current) {
	return snowstar::createProxy<FilterWheelPrx>(
		NameConverter::urlencode(filterwheelname), current);
}

} // namespace snowstar