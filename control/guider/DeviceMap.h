/*
 * DeviceMap.h -- common device naming code
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _DeviceInternals_h
#define _DeviceInternals_h

#include <ObjWrapper.h>
#include <AstroDebug.h>
#include <string>
#include <map>

namespace astro {
namespace cli {

/**
 * \brief map class for device references
 *
 * The cli interface can talk to all types of devices, and for each device
 * type it has a map to assign names to devices. All these classes derive
 * from the DeviceMap template. Only the device specific methods,
 * in particular the construction of a reference from an argument list
 * needs to be implemented in the derived classes.
 */

template<typename AstroDevice>
class DeviceMap : public std::map<std::string, ObjWrapper<AstroDevice> > {
public:
	typedef	ObjWrapper<AstroDevice>	DeviceWrapper;
	typedef std::map<std::string, ObjWrapper<AstroDevice> >	maptype;
	typedef typename std::map<std::string, ObjWrapper<AstroDevice> >::iterator	iterator;
	typedef typename std::map<std::string, ObjWrapper<AstroDevice> >::value_type	value_type;
	DeviceWrapper	byname(const std::string& deviceid);
	void	release(const std::string& deviceid);
protected:
	void	assign(const std::string& deviceid,
			typename AstroDevice::_ptr_type device);
public:
	virtual void	assign(const std::string& deviceid,
			const std::vector<std::string>& arguments) = 0;
};

template<typename AstroDevice>
void	DeviceMap<AstroDevice>::release(const std::string& deviceid) {
	iterator	i = maptype::find(deviceid);
	if (i != maptype::end()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "removing device %s",
			deviceid.c_str());
		maptype::erase(i);
	}
}

template<typename AstroDevice>
void	DeviceMap<AstroDevice>::assign(const std::string& deviceid,
		typename AstroDevice::_ptr_type device) {
	value_type	v(deviceid, DeviceWrapper(device));
	maptype::insert(v);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "device %s stored in map",
		deviceid.c_str());
}

template<typename AstroDevice>
ObjWrapper<AstroDevice>	DeviceMap<AstroDevice>::byname(const std::string& deviceid) {
	iterator	i = maptype::find(deviceid);
	if (i == maptype::end()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "device %s not found",
			deviceid.c_str());
		throw command_error("device not found");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found device reference '%s'",
		deviceid.c_str());
	return i->second;
}

} // namespace cli
} // namespace astro

#endif /* _DeviceInternals_h */
