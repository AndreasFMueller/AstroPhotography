/*
 * ServiceInstrument.cpp -- instrument implementation
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <ServiceDiscovery.h>
#include <AstroPersistence.h>
#include <AstroConfig.h>
#include <AstroDebug.h>

namespace astro {
namespace discover {

//////////////////////////////////////////////////////////////////////
// InstrumentComponentKey
//////////////////////////////////////////////////////////////////////

class InstrumentComponentKey {
	InstrumentComponent::Type	_type;
	int	_index;
public:
	InstrumentComponent::Type	type() const { return _type; }
	int	index() const { return _index; }
	InstrumentComponentKey(InstrumentComponent::Type type,
		int index) : _type(type), _index(index) {
	}
	bool operator<(const InstrumentComponentKey& other) const;
};

bool	InstrumentComponentKey::operator<(const InstrumentComponentKey& other) const {
	if (_type < other._type) {
		return true;
	}
	if (_type > other._type) {
		return false;
	}
	return _index < other._index;
}


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
// Instrument Backend
//////////////////////////////////////////////////////////////////////

class InstrumentImpl : public Instrument {
	astro::persistence::Database	database;
public:
	InstrumentImpl(const std::string& name);
private:
	int	nComponentOfType(const std::string& name,
			InstrumentComponent::Type type);
	int	add(const std::string& name,
			const InstrumentComponent& component);
	void	update(const std::string& name,
			const InstrumentComponent& component);
	void	remove(const std::string& name,
			InstrumentComponent::Type type, int index);
public:
	virtual int	nComponentOfType(InstrumentComponent::Type type) {
		return nComponentOfType(name(), type);
	}
	virtual int	add(const InstrumentComponent component) {
		return add(name(), component);
	}
	virtual void	update(const InstrumentComponent& component) {
		update(name(), component);
	}
	virtual void	remove(InstrumentComponent::Type type, int index) {
		remove(name(), type, index);
	}
};

InstrumentImpl::InstrumentImpl(const std::string& name) : Instrument(name) {
	astro::config::ConfigurationPtr	config
		= astro::config::Configuration::get();
	database = config->database();
	// XXX make sure the database contains the instrument table
}

int	InstrumentImpl::nComponentOfType(const std::string& name,
			InstrumentComponent::Type type) {
	return 0;
}

int	InstrumentImpl::add(const std::string& name,
			const InstrumentComponent& component) {
	return 0;
}

void	InstrumentImpl::update(const std::string& name,
			const InstrumentComponent& component) {
}

void	InstrumentImpl::remove(const std::string& name,
			InstrumentComponent::Type type, int index) {
}

} // namespace discover
} // namespace astro
