/*
 * DevicesIconversions.cpp -- conversion methods for device access
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <DevicesI.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/Communicator.h>
#include <AstroLoader.h>
#include <NameConverter.h>

namespace snowstar {

DeviceNameList	DevicesI::convert(
	const astro::module::Devices::devicelist& list) {
	DeviceNameList	result;
	astro::module::Devices::devicelist::const_iterator	i;
	for (i = list.begin(); i != list.end(); i++) {
		result.push_back(NameConverter::urlencode(i->toString()));
	}
	return result;
}

devicetype	DevicesI::convert(const astro::DeviceName::device_type& type) {
	switch (type) {
	case astro::DeviceName::AdaptiveOptics:
		return DevAO;
	case astro::DeviceName::Camera:
		return DevCAMERA;
	case astro::DeviceName::Ccd:
		return DevCCD;
	case astro::DeviceName::Cooler:
		return DevCOOLER;;
	case astro::DeviceName::Filterwheel:
		return DevFILTERWHEEL;
	case astro::DeviceName::Focuser:
		return DevFOCUSER;
	case astro::DeviceName::Guiderport:
		return DevGUIDERPORT;
	case astro::DeviceName::Module:
		return DevMODULE;
	case astro::DeviceName::Mount:
		return DevMOUNT;
	}
}

astro::DeviceName::device_type	DevicesI::convert(const devicetype& type) {
	switch (type) {
	case DevAO:
		return astro::DeviceName::AdaptiveOptics;
	case DevCAMERA:
		return astro::DeviceName::Camera;
	case DevCCD:
		return astro::DeviceName::Ccd;
	case DevCOOLER:
		return astro::DeviceName::Cooler;
	case DevFILTERWHEEL:
		return astro::DeviceName::Filterwheel;
	case DevFOCUSER:
		return astro::DeviceName::Focuser;
	case DevGUIDERPORT:
		return astro::DeviceName::Guiderport;
	case DevMODULE:
		return astro::DeviceName::Module;
	case DevMOUNT:
		return astro::DeviceName::Mount;
	}
}

} // namespace snowstar
