/*
 * ServiceInstrument.cpp -- instrument implementation
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <ServiceDiscovery.h>
#include <AstroPersistence.h>
#include <AstroConfig.h>
#include <AstroDebug.h>
#include <InstrumentComponentTable.h>

namespace astro {
namespace discover {

//////////////////////////////////////////////////////////////////////
// Instrument implementation
//////////////////////////////////////////////////////////////////////

void	Instrument::add(std::list<InstrumentComponent>& l,
		InstrumentComponent::Type type) {
	int	n = nComponentsOfType(type);
	for (int i = 0; i < n; i++) {
		l.push_back(get(type, i));
	}
}

std::list<InstrumentComponent>	Instrument::list() {
	std::list<InstrumentComponent>	result;
	add(result, InstrumentComponent::CCD);
	add(result, InstrumentComponent::GuiderCCD);
	add(result, InstrumentComponent::Cooler);
	add(result, InstrumentComponent::GuiderPort);
	add(result, InstrumentComponent::Focuser);
	add(result, InstrumentComponent::AdaptiveOptics);
	return result;
}

std::list<InstrumentComponent>	Instrument::list(InstrumentComponent::Type type) {
	std::list<InstrumentComponent>	result;
	add(result, type);
	return result;
}

//////////////////////////////////////////////////////////////////////
// Instrument Backend implementation
//////////////////////////////////////////////////////////////////////
class InstrumentBackendImpl {
	// shared database connection
	static astro::persistence::Database	database;
	static InstrumentComponentTablePtr	table;
	static void	setup() {
		astro::config::ConfigurationPtr	config
			= astro::config::Configuration::get();
		database = config->database();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "create table");
		table = InstrumentComponentTablePtr(
				new InstrumentComponentTable(database));
	}
	static std::once_flag	ready;
public:
	InstrumentBackendImpl() {
		std::call_once(ready, setup);
	}
	int	idfromkey(const InstrumentComponentKey& key);
	int	nComponentsOfType(const std::string& name,
			InstrumentComponent::Type type);
	int	add(const std::string& name,
			const InstrumentComponent& component);
	void	update(const std::string& name,
			const InstrumentComponent& component);
	void	remove(const std::string& name,
			InstrumentComponent::Type type, int index);
	void	remove(const InstrumentComponentKey& key);
	std::list<std::string>	names();
	InstrumentPtr	get(const std::string& name);
};

astro::persistence::Database	InstrumentBackendImpl::database;
InstrumentComponentTablePtr	InstrumentBackendImpl::table;
std::once_flag	InstrumentBackendImpl::ready;

int	InstrumentBackendImpl::nComponentsOfType(const std::string& name,
			InstrumentComponent::Type type) {
	return 0;
}

int	InstrumentBackendImpl::add(const std::string& name,
			const InstrumentComponent& component) {
	return 0;
}

void	InstrumentBackendImpl::update(const std::string& name,
			const InstrumentComponent& component) {
}

void	InstrumentBackendImpl::remove(const std::string& name,
			InstrumentComponent::Type type, int index) {
	
}

std::list<std::string>	InstrumentBackendImpl::names() {
	std::list<std::string>	result;
	std::string	query(	"select distinct name "
				"from instrumentcomponents "
				"order by 1 asc;");
	astro::persistence::Result	r = database->query(query);
	astro::persistence::Result::iterator	i;
	for (i = r.begin(); i != r.end(); i++) { 
		result.push_back((*i)[0]->stringValue());
	}
	return result;
}

//////////////////////////////////////////////////////////////////////
// Instrument implementation
//////////////////////////////////////////////////////////////////////
class InstrumentImpl : public Instrument {
	InstrumentBackendImpl	backend;
public:
	InstrumentImpl(const std::string& name) : Instrument(name) { }
public:
	virtual int	nComponentsOfType(InstrumentComponent::Type type) {
		return backend.nComponentsOfType(name(), type);
	}
	virtual int	add(const InstrumentComponent& component) {
		return backend.add(name(), component);
	}
	virtual void	update(const InstrumentComponent& component) {
		backend.update(name(), component);
	}
	virtual void	remove(InstrumentComponent::Type type, int index) {
		backend.remove(name(), type, index);
	}
};

InstrumentPtr	InstrumentBackendImpl::get(const std::string& name) {
	return InstrumentPtr(new InstrumentImpl(name));
}

//////////////////////////////////////////////////////////////////////
// Instrument Backend
//////////////////////////////////////////////////////////////////////

std::list<std::string>	InstrumentBackend::names() {
	InstrumentBackendImpl	backend;
	return backend.names();
}

InstrumentPtr	InstrumentBackend::get(const std::string& name) {
	InstrumentBackendImpl	backend;
	return backend.get(name);
}

} // namespace discover
} // namespace astro
