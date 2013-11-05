/*
 * NetUtils.cpp --
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <NetUtils.h>
#include <AstroUtils.h>

namespace astro {
namespace camera {
namespace net {

DeviceName	devname2netname(const DeviceName& devname) {
	std::string	unitname = devname;
	return devname2netname(unitname);
}

DeviceName	devname2netname(const std::string& name) {
	DeviceName	netname("net", URL::encode(name));
	netname.type(netname.type());
	return netname;
}

DeviceName	netname2devname(const DeviceName& netname) {
	DeviceName	devname(URL::decode(devname.unitname()));
	return devname;
}

} // namespace net
} // namespace camera
} // namespace astro
