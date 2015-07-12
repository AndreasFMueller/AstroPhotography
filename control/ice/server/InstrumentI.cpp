/*
 * InstrumentI.cpp -- 
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <InstrumentI.h>
#include <IceConversions.h>

namespace snowstar {

InstrumentI::InstrumentI(astro::discover::InstrumentPtr instrument) : _instrument(instrument) {
}

InstrumentI::~InstrumentI() {
}

std::string	InstrumentI::name(const Ice::Current& /* current */) {
	return _instrument->name();
}

int	InstrumentI::nComponentsOfType(InstrumentComponentType type,
		const Ice::Current& /* current */) {
	return _instrument->nComponentsOfType((astro::discover::InstrumentComponentKey::Type)type);
}

InstrumentComponent	InstrumentI::getComponent(InstrumentComponentType type,
		int index, const Ice::Current& /* current */) {
	return convert(_instrument->get((astro::discover::InstrumentComponentKey::Type)type, index));
}

int	InstrumentI::add(const InstrumentComponent& component,
		const Ice::Current& /* current */) {
	return _instrument->add(convert(component));
}

void	InstrumentI::update(const InstrumentComponent& component,
		const Ice::Current& /* current */) {
	_instrument->update(convert(component));
}

void	InstrumentI::remove(InstrumentComponentType type, int index,
		const Ice::Current& /* current */) {
	_instrument->remove((astro::discover::InstrumentComponentKey::Type)type, index);
}

InstrumentComponentList	InstrumentI::list(const Ice::Current& /* current */) {
	return convert(_instrument->list());
}

} // namespace snowstar
