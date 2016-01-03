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

astro::discover::InstrumentComponentKey::Type	convertInstrumentType(
	const InstrumentComponentType type) {
	switch (type) {
	case InstrumentAdaptiveOptics:
		return astro::discover::InstrumentComponentKey::AdaptiveOptics;
	case InstrumentCamera:
		return astro::discover::InstrumentComponentKey::Camera;
	case InstrumentCCD:
		return astro::discover::InstrumentComponentKey::CCD;
	case InstrumentCooler:
		return astro::discover::InstrumentComponentKey::Cooler;
	case InstrumentGuiderCCD:
		return astro::discover::InstrumentComponentKey::GuiderCCD;
	case InstrumentGuiderPort:
		return astro::discover::InstrumentComponentKey::GuiderPort;
	case InstrumentFilterWheel:
		return astro::discover::InstrumentComponentKey::FilterWheel;
	case InstrumentFocuser:
		return astro::discover::InstrumentComponentKey::Focuser;
	case InstrumentMount:
		return astro::discover::InstrumentComponentKey::Mount;
	}
	throw std::runtime_error("bad type");
}

InstrumentComponentType	convertInstrumentType(
	const astro::discover::InstrumentComponentKey::Type type) {
	switch (type) {
	case astro::discover::InstrumentComponentKey::AdaptiveOptics:
		return InstrumentAdaptiveOptics;
	case astro::discover::InstrumentComponentKey::Camera:
		return InstrumentCamera;
	case astro::discover::InstrumentComponentKey::CCD:
		return InstrumentCCD;
	case astro::discover::InstrumentComponentKey::Cooler:
		return InstrumentCooler;
	case astro::discover::InstrumentComponentKey::GuiderCCD:
		return InstrumentGuiderCCD;
	case astro::discover::InstrumentComponentKey::GuiderPort:
		return InstrumentGuiderPort;
	case astro::discover::InstrumentComponentKey::FilterWheel:
		return InstrumentFilterWheel;
	case astro::discover::InstrumentComponentKey::Focuser:
		return InstrumentFocuser;
	case astro::discover::InstrumentComponentKey::Mount:
		return InstrumentMount;
	}
	throw std::runtime_error("bad type");
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


std::string	instrumentIndex2name(const std::string& instrumentname,
			InstrumentComponentType type, int index) {
	// first try to look up the device in the instrument database
	try {
		// get an instrument for the instrument name
		astro::discover::InstrumentPtr  instrument
			= astro::discover::InstrumentBackend::get(
				instrumentname);
		return instrument->get(convertInstrumentType(type),
			index).deviceurl();
	} catch (...) {
	}
	// build an unknown name from the index
	std::string	devicename;
	switch (type) {
	case astro::discover::InstrumentComponentKey::AdaptiveOptics:
		devicename = "adaptiveoptics";
		break;
	case astro::discover::InstrumentComponentKey::Camera:
		devicename = "camera";
		break;
	case astro::discover::InstrumentComponentKey::CCD:
	case astro::discover::InstrumentComponentKey::GuiderCCD:
		devicename = "ccd";
		break;
	case astro::discover::InstrumentComponentKey::Cooler:
		devicename = "camera";
		break;
	case astro::discover::InstrumentComponentKey::GuiderPort:
		devicename = "guiderport";
		break;
	case astro::discover::InstrumentComponentKey::FilterWheel:
		devicename = "filterwheel";
		break;
	case astro::discover::InstrumentComponentKey::Focuser:
		devicename = "focuser";
		break;
	case astro::discover::InstrumentComponentKey::Mount:
		devicename = "mount";
		break;
	default:
		throw std::runtime_error("unknown device type");
		break;
	}
	std::string deviceurl = astro::stringprintf("%s:unknown/%d",
		devicename.c_str(), index);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "unknown device: %s", deviceurl.c_str());
	return deviceurl;
}

int	instrumentName2index(const std::string& instrumentname,
			InstrumentComponentType type,
			const std::string& deviceurl) {
	// handle the case where the device url comes from the "unknown" module
	astro::DeviceName	devname(deviceurl);
	if (devname.modulename() == "unknown") {
		try {
			return std::stoi(devname.unitname());
		} catch (...) {
			return 0;
		}
	}
	// handle all other device names, try to convert them into an index
	try {
		astro::discover::InstrumentPtr  instrument
			= astro::discover::InstrumentBackend::get(instrumentname);
		int	index = instrument->indexOf(convertInstrumentType(type),
					deviceurl);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s has index %d in %s",
			deviceurl.c_str(), index, instrumentname.c_str());
	} catch (...) { }
	// return 0 if this is unknown
	return 0;
}

} // namespace snowstar
