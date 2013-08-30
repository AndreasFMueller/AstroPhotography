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

/**
 * \brief Singleton class to keep a reference to the ORB
 *
 * The net module needs access to the ORB as a client, but will not
 * have a connection to the context where the ORB was initialized, so
 * we use this class to hold the reference to the ORB.
 */
class OrbSingleton {
	CORBA::ORB_var	_orbvar;
public:
	OrbSingleton(int& argc, char *argv[]);
	OrbSingleton();
	~OrbSingleton();
	CORBA::ORB_var	orbvar() { return _orbvar; }
	operator	CORBA::ORB_var() { return _orbvar; }
	Modules_var	getModules();
	DeviceLocator_var       getDeviceLocator(const std::string& modulename);
};

} // namespace Astro

#endif /* _OrbSingleton_h */
