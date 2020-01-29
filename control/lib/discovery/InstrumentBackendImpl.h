/*
 * InstrumentBackendImpl.h -- instrument backend implementation
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _InstrumentBackendImpl_h
#define _InstrumentBackendImpl_h

#include <AstroDiscovery.h>
#include <AstroPersistence.h>
#include <AstroConfig.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <InstrumentComponentTable.h>
#include <InstrumentPropertyTable.h>

namespace astro {
namespace discover {

class InstrumentBackendImpl {
	// shared database connection
	static astro::persistence::Database	database;
	static InstrumentComponentTablePtr	components;
	static InstrumentPropertyTablePtr	properties;
	static void	setup() {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "setup of with default db");
		astro::config::ConfigurationPtr	config
			= astro::config::Configuration::get();
		database = config->database();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "get handle to table");
		components = InstrumentComponentTablePtr(
				new InstrumentComponentTable(database));
		properties = InstrumentPropertyTablePtr(
				new InstrumentPropertyTable(database));
	}
	static std::once_flag	ready;
public:
	InstrumentBackendImpl() {
		std::call_once(ready, setup);
	}
	// instrument component methods
	long	idfromkey(const InstrumentComponentKey& key);
	long	idfromkey(const std::string& name,
			InstrumentComponent::Type type, int index);
	int	nComponentsOfType(const std::string& name,
			InstrumentComponent::Type type);
	bool	has(const std::string& instrumentname);
	int	add(const InstrumentComponent& component);
	void	update(const InstrumentComponent& component);
	void	remove(const std::string& name,
			InstrumentComponent::Type type, int index);
	void	remove(const InstrumentComponentKey& key);
	void	remove(const std::string& name);
	std::list<std::string>	names();
	InstrumentPtr	get(const std::string& name);
	InstrumentComponent	get(const std::string& name, 
			InstrumentComponent::Type type, int index);
	int	indexOf(const std::string& instrumentname,
			InstrumentComponent::Type type,
			const std::string& devicename);
	// property methods
private:
	long	propertyid(const std::string& instrumentname,
				const std::string& property);
public:
	long	addProperty(const InstrumentProperty& property);
	bool	hasProperty(const std::string& instrumentname,
			const std::string& property);
	InstrumentProperty	getProperty(const std::string& instrumentname,
				const std::string& property);
	void	removeProperty(const std::string& instrumentname,
				const std::string& property);
	void	updateProperty(const InstrumentProperty& property);
	Instrument::PropertyNames	getPropertyNames(const std::string& instrumentname);
	InstrumentPropertyList	getProperties(const std::string& instrumentname);
};

} // namespace discover
} // namespace astro

#endif /* _InstrumentBackendImpl_h */
