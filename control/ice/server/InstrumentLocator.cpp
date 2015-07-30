/*
 * InstrumentLocator.cpp -- InstrumentLocator implementation
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <InstrumentLocator.h>
#include <AstroDiscovery.h>
#include <InstrumentI.h>

namespace snowstar {

InstrumentLocator::InstrumentLocator() {
}

Ice::ObjectPtr	InstrumentLocator::locate(const Ice::Current& current,
			Ice::LocalObjectPtr& /* cookie */) {
	std::string	name = current.id.name;

	// check whether we already have an Instrument of this name
	instrumentmap::iterator	i = instruments.find(name);
	if (i != instruments.end()) {
		return i->second;
	}

	// we have to create a new servant
	astro::discover::InstrumentPtr	instrument
		= astro::discover::InstrumentBackend::get(name);
	Ice::ObjectPtr	ptr = new InstrumentI(instrument);
	instruments.insert(std::make_pair(name, ptr));
	return ptr;
}

void	InstrumentLocator::finished(const Ice::Current& /* current */,
		const Ice::ObjectPtr& /* servant */,
		const Ice::LocalObjectPtr& /* cookie */) {
}

void	InstrumentLocator::deactivate(const std::string& /* category */) {
}

} // namespace snowstar
