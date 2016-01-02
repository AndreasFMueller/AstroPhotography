/*
 * InstrumentI.cpp -- 
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <InstrumentI.h>
#include <IceConversions.h>
#include <AstroFormat.h>
#include <AstroDebug.h>

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
	return _instrument->nComponentsOfType(
		(astro::discover::InstrumentComponentKey::Type)type);
}

InstrumentComponent	InstrumentI::getComponent(InstrumentComponentType type,
		int index, const Ice::Current& /* current */) {
	try {
		astro::discover::InstrumentComponent	component
			= _instrument->get(
			(astro::discover::InstrumentComponentKey::Type)type,
			index);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found component: %s",
			component.toString().c_str());
		return convert(component);
	} catch (const std::exception& x) {
		std::string	cause = astro::stringprintf(
			"cannot find %s[%d]: %s",
			instrumentcomponent2name(type).c_str(),
			index, x.what());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", cause.c_str());
		throw NotFound(cause);
	}
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
