/*
 * InstrumentsI.cpp -- instruments implementation
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <InstrumentsI.h>
#include <ProxyCreator.h>
#include <AstroDiscovery.h>
#include <AstroDebug.h>
#include <IceConversions.h>

namespace snowstar {

InstrumentsI::InstrumentsI() {
}

InstrumentsI::~InstrumentsI() {
}

InstrumentPrx	InstrumentsI::get(const std::string& name,
			const Ice::Current& current) {
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "request instrument '%s'", name.c_str());

	return createProxy<InstrumentPrx>("instrument/" + name, current,
		false);
}

void	InstrumentsI::remove(const std::string& name,
		const Ice::Current& current) {
	CallStatistics::count(current);
	astro::discover::InstrumentBackend::remove(name);
}

InstrumentList	InstrumentsI::list(const Ice::Current& current) {
	CallStatistics::count(current);
	return convert(astro::discover::InstrumentBackend::names());
}

bool	InstrumentsI::has(const std::string& name,
		const Ice::Current& current) {
	CallStatistics::count(current);
	return astro::discover::InstrumentBackend::has(name);
}


} // namespace snowstar
