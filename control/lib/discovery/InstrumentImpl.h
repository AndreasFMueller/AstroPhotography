/*
 * InstrumentImpl.h -- instrument implementation
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _InstrumentImpl_h
#define _InstrumentImpl_h

#include "InstrumentBackendImpl.h"
#include <AstroDiscovery.h>
#include <AstroPersistence.h>
#include <AstroConfig.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <InstrumentComponentTable.h>
#include <InstrumentPropertyTable.h>

namespace astro {
namespace discover {

class InstrumentImpl : public Instrument {
	InstrumentBackendImpl	backend;
public:
	InstrumentImpl(const std::string& name) : Instrument(name) { }
	virtual ~InstrumentImpl() { }
public:
	virtual int	nComponentsOfType(InstrumentComponent::Type type) {
		return backend.nComponentsOfType(name(), type);
	}
	virtual int	add(const InstrumentComponent& component) {
		return backend.add(component);
	}
	virtual void	update(const InstrumentComponent& component) {
		backend.update(component);
	}
	virtual void	remove(InstrumentComponent::Type type, int index) {
		backend.remove(name(), type, index);
	}
	virtual InstrumentComponent	get(InstrumentComponent::Type type,
		int index) {
		return backend.get(name(), type, index);
	}
	virtual int	indexOf(InstrumentComponent::Type type,
				const std::string& deviceurl) {
		return backend.indexOf(name(), type, deviceurl);
	}
	// property methods
	virtual int	addProperty(const InstrumentProperty& property) {
		return backend.addProperty(property);
	}
	virtual InstrumentProperty	getProperty(const std::string& property) {
		return backend.getProperty(name(), property);
	}
	virtual bool	hasProperty(const std::string& property) {
		return backend.hasProperty(name(), property);
	}
	virtual void	removeProperty(const std::string& property) {
		backend.removeProperty(name(), property);
	}
	virtual void	updateProperty(const InstrumentProperty& property) {
		backend.updateProperty(property);
	}
	virtual PropertyNames	getPropertyNames() {
		return backend.getPropertyNames(name());
	}
	virtual InstrumentPropertyList	getProperties() {
		return backend.getProperties(name());
	}
};

} // namespace discover
} // namespace astro

#endif /* _InstrumentImpl_h */
