/*
 * InstrumentsI.cpp -- instruments implementation
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <InstrumentsI.h>
#include <ProxyCreator.h>
#include <ServiceDiscovery.h>
#include <AstroDebug.h>
#include <IceConversions.h>

namespace snowstar {

InstrumentsI::InstrumentsI() {
}

InstrumentsI::~InstrumentsI() {
}

InstrumentPrx	InstrumentsI::get(const std::string& name,
			const Ice::Current& current) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "request instrument '%s'", name.c_str());

	return createProxy<InstrumentPrx>("instrument/" + name, current,
		false);
}

void	InstrumentsI::remove(const std::string& name,
		const Ice::Current& /* current */) {
	astro::discover::InstrumentBackend::remove(name);
}

InstrumentList	InstrumentsI::list(const Ice::Current& /* current */) {
	return convert(astro::discover::InstrumentBackend::names());
}

} // namespace snowstar
