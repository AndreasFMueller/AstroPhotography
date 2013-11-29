/*
 * OrbSingleton.h -- wrapper around the ORB stuff
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _OrbSingleton_h
#define _OrbSingleton_h

#include <module.hh>
#include <guider.hh>
#include <image.hh>
#include <string>
#include <vector>

namespace Astro {

/**
 * \brief An abstraction for the hierarchical naming of POAs
 */
class PoaName : public std::vector<std::string> {
public:
	PoaName(const std::string& basename);
	PoaName&	add(const std::string& name);
	std::string	toString() const;
	static PoaName	modules();
	static PoaName	drivermodules();
	static PoaName	cameras();
	static PoaName	guiderports();
	static PoaName	filterwheels();
	static PoaName	coolers();
	static PoaName	ccds();
	static PoaName	focusers();
	static PoaName	guiders();
};

std::ostream&	operator<<(std::ostream& out, const PoaName& poaname);

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
	Images_var	getImages();
	GuiderFactory_var	getGuiderfactory();
	DeviceLocator_var       getDeviceLocator(const std::string& modulename);
	PortableServer::POA_var	findPOA(const std::vector<std::string>& poaname);
};

} // namespace Astro

#endif /* _OrbSingleton_h */
