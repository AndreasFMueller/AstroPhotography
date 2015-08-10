/*
 * AdaptiveOpticsI.cpp -- ICE AdaptiveOptics wrapper implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AdaptiveOpticsI.h>
#include <GuiderPortI.h>
#include <NameConverter.h>
#include <IceConversions.h>

namespace snowstar {

AdaptiveOpticsI::AdaptiveOpticsI(astro::camera::AdaptiveOpticsPtr ao)
	: DeviceI(*ao), _ao(ao) {
}

AdaptiveOpticsI::~AdaptiveOpticsI() {
}

void	AdaptiveOpticsI::set(const Point& position, const Ice::Current& /* current */) {
	_ao->set(convert(position));
}

Point	AdaptiveOpticsI::get(const Ice::Current& /* current */) {
	return convert(_ao->get());
}

bool	AdaptiveOpticsI::hasGuiderPort(const Ice::Current& /* current */) {
	return _ao->hasGuiderPort();
}

GuiderPortPrx	AdaptiveOpticsI::getGuiderPort(const Ice::Current& current) {
	std::string	name
		= NameConverter::urlencode(_ao->getGuiderPort()->name());
	return GuiderPortI::createProxy(name, current);
}

void	AdaptiveOpticsI::center(const Ice::Current& /* current */) {
	_ao->center();
}

} // namespace snowstar
