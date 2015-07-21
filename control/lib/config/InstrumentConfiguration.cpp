/*
 * Configuration.cpp
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroConfig.h>
#include <AstroPersistence.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <cstdlib>
#include "InstrumentTables.h"

using namespace astro::persistence;
using namespace astro::project;

namespace astro {
namespace config {

/**
 * \brief configuration backend
 *
 * This is used to hide the fact that there 
 */
class InstrumentConfigurationBackend : public InstrumentConfiguration {
	ConfigurationPtr	_config;
public:
	InstrumentConfigurationBackend(ConfigurationPtr config)
		: _config(config) { }

	// instrument access
	virtual InstrumentPtr	instrument(const std::string& name);
	virtual void    addInstrument(InstrumentPtr instrument);
	virtual void    removeInstrument(const std::string& name);
	virtual std::list<InstrumentPtr>   listinstruments();
};

//////////////////////////////////////////////////////////////////////
// InstrumentConfiguration implementation (static methods)
//////////////////////////////////////////////////////////////////////
InstrumentConfigurationPtr	InstrumentConfiguration::get() {
	return InstrumentConfigurationPtr(
		new InstrumentConfigurationBackend(Configuration::get()));
}

InstrumentConfigurationPtr	InstrumentConfiguration::get(ConfigurationPtr config) {
	return InstrumentConfigurationPtr(
		new InstrumentConfigurationBackend(config));
}

//////////////////////////////////////////////////////////////////////
// Instrument access
//////////////////////////////////////////////////////////////////////
/**
 * \brief Retrieve an Instrument from the database
 */
InstrumentPtr	InstrumentConfigurationBackend::instrument(const std::string& name) {
	// find the id
	InstrumentTable	instruments(_config->database());
	int	instrumentid = instruments.id(name);

	// retrieve the instrument record
	InstrumentRecord	instrumentrecord
					= instruments.byid(instrumentid);
	InstrumentPtr	instrument(new Instrument(_config->database(),
				instrumentrecord.name));

	// return the instrument
	return instrument;
}

/**
 * \brief Convert a InstrumentComponentPtr to an InstrumentComponentRecord
 */
static InstrumentComponentRecord	componentrecord(long instrumentid,
					InstrumentComponentPtr& component) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adding component %s",
		component->name().c_str());
	InstrumentComponentRecord	componentrecord(-1, instrumentid);

	// assign the various members
	componentrecord.unit = component->unit();
	componentrecord.componenttype
		= InstrumentComponentTableAdapter::component_type(
			component->component_type());
	componentrecord.type
		= InstrumentComponentTableAdapter::type(
			component->type());
	componentrecord.devicename = component->name();
	if (component->component_type() == InstrumentComponent::direct) {
		componentrecord.servername = component->servername();
	} else {
		componentrecord.servername = "";
	}

	// that's it, return the record
	return componentrecord;
}

/**
 * \brief Add an instrument to the database
 */
void	InstrumentConfigurationBackend::addInstrument(InstrumentPtr instrument) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "add instrument '%s' to the database",
		instrument->name().c_str());

	// open a transaction bracket
	_config->database()->begin("addinstrument");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "transaction opened");

	try {
		// create an instrument entry
		InstrumentTable	instruments(_config->database());
		InstrumentRecord	instrumentrecord;
		instrumentrecord.name = instrument->name();
		long	instrumentid = instruments.add(instrumentrecord);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "id of new instrument: %d",
			instrumentid);

		// for each component type, create an entry if the type
		// is present
		InstrumentComponentTable	components(_config->database());
		std::list<DeviceName::device_type>	types
			= instrument->component_types();
		for (auto ptr = types.begin(); ptr != types.end(); ptr++) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "component of type %d",
				*ptr);
			DeviceName::device_type	devtype = *ptr;
			InstrumentComponentPtr	cptr
				= instrument->component(devtype);
			components.add(componentrecord(instrumentid, cptr));
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "entry complete");

		// commit the additions
		_config->database()->commit("addinstrument");
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "failed to add '%s': %s",
			instrument->name().c_str(), x.what());
		_config->database()->rollback("addinstrument");
		throw;
	}
}


/**
 * \brief Remove an instrument from the tables
 */
void	InstrumentConfigurationBackend::removeInstrument(const std::string& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "remove instrument named '%s'",
		name.c_str());
	InstrumentTable	instruments(_config->database());
	long	instrumentid = instruments.id(name);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "delete instrument id = %ld",
		instrumentid);
	instruments.remove(instrumentid);
}

/**
 * \brief List all instruments in the database
 */
std::list<InstrumentPtr>	InstrumentConfigurationBackend::listinstruments() {
	std::list<InstrumentPtr>	result;
	InstrumentTable	instruments(_config->database());
	std::list<InstrumentRecord>	records = instruments.select("0 = 0");
	for (auto ptr = records.begin(); ptr != records.end(); ptr++) {
		result.push_back(instrument(ptr->name));
	}
	return result;
}

} // namespace config
} // namespace astro
