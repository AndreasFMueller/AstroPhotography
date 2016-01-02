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

std::string	instrumentcomponent2name(const InstrumentComponentType type) {
	switch (type) {
	case InstrumentAdaptiveOptics:
                return std::string("AdaptiveOptics");
	case InstrumentCamera:
                return std::string("Camera");
	case InstrumentCCD:
                return std::string("CCD");
	case InstrumentCooler:
                return std::string("Cooler");
	case InstrumentGuiderCCD:
                return std::string("GuiderCCD");
	case InstrumentGuiderPort:
                return std::string("GuiderPort");
	case InstrumentFilterWheel:
                return std::string("FilterWheel");
	case InstrumentFocuser:
                return std::string("Focuser");
	case InstrumentMount:
                return std::string("Mount");
	}
	throw std::runtime_error("bad type");
}

InstrumentComponentType	name2instrumentcomponent(const std::string& name) {
	if (std::string("AdaptiveOptics") == name) {
		return InstrumentAdaptiveOptics;
	}
	if (std::string("Camera") == name) {
		return InstrumentCamera;
	}
	if (std::string("CCD") == name) {
		return InstrumentCCD;
	}
	if (std::string("Cooler") == name) {
		return InstrumentCooler;
	}
	if (std::string("GuiderCCD") == name) {
		return InstrumentGuiderCCD;
	}
	if (std::string("GuiderPort") == name) {
		return InstrumentGuiderPort;
	}
	if (std::string("FilterWheel") == name) {
		return InstrumentFilterWheel;
	}
	if (std::string("Focuser") == name) {
		return InstrumentFocuser;
	}
	if (std::string("Mount") == name) {
		return InstrumentMount;
	}
	std::string	msg = astro::stringprintf("unknown instrument "
		"component name: %s", name.c_str());
	throw std::runtime_error(msg);
}

} // namespace snowstar
