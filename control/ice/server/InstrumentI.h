/*
 * InstrumentI.h -- ICE Interface wrapper class definition 
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _InstrumentI_h
#define _InstrumentI_h

#include <instruments.h>
#include <AstroDiscovery.h>

namespace snowstar {

class InstrumentI : virtual public Instrument {
	astro::discover::InstrumentPtr	_instrument;
public:
	InstrumentI(astro::discover::InstrumentPtr instrument);
	virtual ~InstrumentI();

	std::string	name(const Ice::Current& current);

	// component interface
	int	nComponentsOfType(InstrumentComponentType type,
			const Ice::Current& current);
	InstrumentComponent	getComponent(InstrumentComponentType cmoponent,
					int index, const Ice::Current& current);
	int	add(const InstrumentComponent& component,
			const Ice::Current& current);
	void	update(const InstrumentComponent& component,
			const Ice::Current& current);
	void	remove(InstrumentComponentType type, int index,
			const Ice::Current& current);
	InstrumentComponentList	list(const Ice::Current& current);

	// property interface
	void	addProperty(const InstrumentProperty& property,
			const Ice::Current& current);
	InstrumentProperty	getProperty(const std::string& property,
			const Ice::Current& current);
	void	removeProperty(const std::string& property,
			const Ice::Current& current);
	void	updateProperty(const InstrumentProperty& property,
			const Ice::Current& current);
	InstrumentPropertyNames	getPropertyNames(const Ice::Current& current);
	InstrumentPropertyList	getProperties(const Ice::Current& current);

	// other stuff
static	InstrumentPrx	createProxy(const std::string& name,
				const Ice::Current& current);
};

} // namespace snowstar

#endif /* _InstrumentI_h */
