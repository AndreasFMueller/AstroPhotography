/*
 * NiceDevice.cpp -- Base class for Nice devices
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <NiceDevice.h>

using namespace astro;

namespace astro {
namespace camera {
namespace nice {

NiceDevice::NiceDevice(const DeviceName& devicename) {
	astro::device::nice::DeviceDenicer	denicer(devicename);
	_service = denicer.service();
	DeviceName	*d = new DeviceName(denicer.devicename());
	_localname = std::shared_ptr<DeviceName>(d);
}

NiceDevice::~NiceDevice() {
}

DeviceName	NiceDevice::nice(const DeviceName& name) {
	astro::device::nice::DeviceNicer	nicer(service());
	return nicer(name);
}

} // namespace nice
} // namespace camera
} // namespace astro
