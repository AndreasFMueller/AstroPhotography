/*
 * OrbSingleton.h -- wrapper around the ORB stuff
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _OrbSingleton_h
#define _OrbSingleton_h

#include <device.hh>
#include <string>

namespace Astro {

class OrbSingleton {
	CORBA::ORB_var	orbvar;
public:
	OrbSingleton(int argc, char *argv[]);
	OrbSingleton();
	~OrbSingleton();
	Modules_var	getModules();
	DeviceLocator_var       getDeviceLocator(const std::string& modulename);
};

} // namespace Astro

#endif /* _OrbSingleton_h */
