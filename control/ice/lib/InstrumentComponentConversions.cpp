/*
 * InstrumentConversions.cpp -- Conversions for instrument related objects
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <IceConversions.h>

using namespace astro;

namespace snowstar {

discover::InstrumentComponent	convert(const struct InstrumentComponent& component) {
	discover::InstrumentComponentKey	key(component.instrumentname,
		(discover::InstrumentComponentKey::Type)component.type,
		(int)component.index);
	discover::InstrumentComponent	c(key, component.servicename,
		component.deviceurl);
	return c;
}

struct InstrumentComponent	convert(const discover::InstrumentComponent& component) {
	InstrumentComponent	c;
	c.instrumentname = component.name();
	c.type = (InstrumentComponentType)component.type();
	c.index = component.index();
	c.servicename = component.servicename();
	c.deviceurl = component.deviceurl();
	return c;
}

template<class From, class To>
To	convert(const From& list) {
	To	result;
	typename From::const_iterator	i;
	for (i = list.begin(); i != list.end(); i++) {
		result.push_back(convert(*i));
	}
	return result;
}

template<class From, class To>
To	copy(const From& list) {
	To	result;
	typename From::const_iterator	i;
	for (i = list.begin(); i != list.end(); i++) {
		result.push_back(*i);
	}
	return result;
}

InstrumentComponentList	convert(const discover::Instrument::ComponentList& list) {
	return convert<discover::Instrument::ComponentList, InstrumentComponentList>(list);
}

discover::Instrument::ComponentList	convert(const InstrumentComponentList& list) {
	return convert<InstrumentComponentList, discover::Instrument::ComponentList>(list);
}

InstrumentList	convert(const discover::InstrumentList& list) {
	return copy<discover::InstrumentList, InstrumentList>(list);
}

discover::InstrumentList	convertInstrumentList(const InstrumentList& list) {
	return copy<InstrumentList, discover::InstrumentList>(list);
}

} // namespace snowstar
