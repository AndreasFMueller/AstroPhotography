/*
 * DeviceConversions.cpp -- conversions between ice and astro
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <IceConversions.h>
#include <includes.h>
#include <AstroIO.h>
#include <AstroUtils.h>

namespace snowstar {

// Device conversions
DeviceNameList	convert(const astro::module::Devices::devicelist& list) {
	DeviceNameList	result;
	for (auto ptr = list.begin(); ptr != list.end(); ptr++) {
		result.push_back(ptr->toString());
	}
	return result;
}

astro::module::Devices::devicelist	convert(const DeviceNameList& list) {
	astro::module::Devices::devicelist	result;
	for (auto ptr = list.begin(); ptr != list.end(); ptr++) {
		result.push_back(astro::DeviceName(*ptr));
	}
	return result;
}

devicetype	convert(const astro::DeviceName::device_type& type) {
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
	case astro::DeviceName::Guideport:
		return DevGUIDEPORT;
	case astro::DeviceName::Module:
		return DevMODULE;
	case astro::DeviceName::Mount:
		return DevMOUNT;
	}
	throw std::runtime_error("unknown device type");
}

astro::DeviceName::device_type	convert(const devicetype& type) {
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
	case DevGUIDEPORT:
		return astro::DeviceName::Guideport;
	case DevMODULE:
		return astro::DeviceName::Module;
	case DevMOUNT:
		return astro::DeviceName::Mount;
	}
	throw std::runtime_error("unknown device type");
}

} // namespace snowstar
