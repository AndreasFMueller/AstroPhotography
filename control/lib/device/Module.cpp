/*
 * Module.cpp -- Module implementation
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroLoader.h>
#include <includes.h>
#include <iostream>
#include <fstream>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroEvent.h>

using namespace astro::device;

namespace astro {
namespace module {

/**
 * \brief Control closing of dynamic library on module close 
 *
 * Under certain conditions, most notably when running CPPUNIT unit
 * tests, closing the dynamic library causes the program to crash.
 * The crash is caused by the ModuleDescriptorPtr returned by getDescriptor
 * being deallocated after the library has been closed. This can be
 * prevented if the either the returned ModuleDescriptorPtr goes out of
 * scope before the library is unloaded, or by turning of unloading
 * of the library completely. This is what dlclose_on_close does.
 * When set to false, the library is not dlclose()d.
 */
bool	Module::dlclose_on_close = true;

/**
 * \brief Read the code filename from the .la file.
 *
 * As usual when using libtool managed libraries, the .la files are used
 * as the handles to the modules, but the dlname attribute found in the
 * .la file specifies the file containing the code. This method
 * scans the .la file for the dlname attribute and returns a fully
 * qualified path to the code file, if it is found.
 *
 * \param lafile	the name of the load file
 */
std::string	Module::getDlname(const std::string& lafile) const {
	// open the file
	debug(LOG_DEBUG, DEBUG_LOG, 0, "reading .la-file %s", lafile.c_str());
	std::ifstream	in(lafile.c_str(), std::ifstream::in);

	// scan the file
	char	linebuffer[1024];
	while (in.good()) {
		in.getline(linebuffer, sizeof(linebuffer));
		if (in.fail() || in.eof()) {
			goto end;
		}
		if (0 == strncmp(linebuffer, "dlname='", 8)) {
			char	*p = linebuffer + 8;
			char	*end = strchr(p, '\'');
			if (!end) {
				throw std::domain_error("corrupt .la file");
			}
			*end = '\0';
			in.close();
			std::string	shared = dirname + "/" + std::string(p);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "shared: %s",
				shared.c_str());
			return shared;
		}
	}
end:
	in.close();
	throw std::domain_error("dlname not found in la file");
}

/**
 * \brief Check for the code file.
 *
 * Before a module can be instantiated, it has to be checked that the code
 * file actually exists, and is accessible by the user. This method
 * use used to encapsulate these checks.
 */
bool	Module::dlfileexists() const {
	// check that we can stast the dlname file
	struct stat	sb;
	if (stat(_dlname.c_str(), &sb) < 0) {
		return false;
	}

	// check that the dlfile is accessible
	if (access(_dlname.c_str(), R_OK) < 0) {
		return false;
	}

	// make sure the dl file is a file
	if (!S_ISREG(sb.st_mode)) {
		return false;
	}

	return true;
}

/**
 * \brief Get the base name of the module file
 */
std::string	Module::basename() const {
	// find the last / in the name
	auto	i = _dlname.find_last_of("/");
	if (i == std::string::npos) {
		return _dlname;
	}
	return _dlname.substr(i + 1);
}

/**
 * \brief Construct a module given the repository directory and the module
 *        name.
 *
 * The file name of the .la file is constructed by concatenating the
 * directory name, the module name and the suffix .la.
 *
 * \param _dirname	name of the directory where the modules can be found
 * \param modulename	the name of the module
 */
Module::Module(const std::string& _dirname, const std::string& modulename)
	: dirname(_dirname), _modulename(modulename) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating module %s",
		_modulename.c_str());
	// NULL handle means module has not been loaded yet
	handle = NULL;

	// get the path to the la file
	_dlname = getDlname(dirname + "/" + _modulename + ".la");

	// get information about the file
	if (dlfileexists()) {
		struct stat	sb;
		if (stat(_dlname.c_str(), &sb) < 0) {
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot get ctime for %s",
				_dlname.c_str());
		} else {
			_ctime.time(sb.st_ctime);
			_size = sb.st_size;
		}
	} else {
		std::string	msg = stringprintf(
			"dl file '%s' not accessible", _dlname.c_str());
	
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::domain_error(msg);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "module %s created from file %s",
		modulename.c_str(), _dlname.c_str());
}

/**
 * \brief Returns the name of the code file (to be) loaded.
 */
const std::string&	Module::filename() const {
	return _dlname;
}

/**
 * \brief Compare modules.
 *
 * \param other		the other module to copy from
 */
bool	Module::operator==(const Module& other) const {
	return (other.dirname == dirname) && (other.modulename() == modulename());
}

/**
 * \brief Open the module by loading and initializing it
 *
 * This method uses dlopen() to load the code file into the address space,
 * initializes the library and keeps a handle to the library for later
 * use. This method must be called before any module functions can be called.
 */
void	Module::open() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "opening module");
	if (isloaded()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "already open");
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "really loading now");

	dlerror(); // clear error conditions

	debug(LOG_DEBUG, DEBUG_LOG, 0, "loading library %s", _dlname.c_str());
	handle = dlopen(_dlname.c_str(), RTLD_NOW);
	if (NULL == handle) {
		std::string	msg = stringprintf("cannot load %s: %s",
			_dlname.c_str(), dlerror());
		event(EVENT_CLASS, astro::events::CRIT,
			astro::events::Event::DEVICE, msg);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	event(EVENT_CLASS, astro::events::NOTICE, astro::events::Event::DEVICE,
		stringprintf("module '%s' loaded", _dlname.c_str()));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "library opened: handle = %p", handle);
}

/**
 * \brief Close a module
 *
 * There is no check that no client is using the module, so closing
 * a module while it is in use will most likely crash the application.
 * See the description of the static variable dlclose_on_close for a
 * way to prevent closing the shared library alltogether.
 */
void	Module::close() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "closing library");
	if ((handle) && (dlclose_on_close)) {
		dlclose(handle);
	}
	handle = NULL;
}

/**
 * \brief Get a pointer to given symbol
 *
 * \param symbolname	the name of the symbol to check for
 */
void	*Module::getSymbol(const std::string& symbolname) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "looking up symbol %s",
		symbolname.c_str());
	// make sure the module is already loaded
	if (!isloaded()) {
		this->open();
	}

	// find the symbol for the getDescriptor function
	void	*s = dlsym(handle, symbolname.c_str());
	if (NULL == s) {
		std::string	msg = stringprintf("module %s lacks symbol %s",
			_modulename.c_str(), symbolname.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::invalid_argument(msg);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "symbol found at %p", s);

	return s;
}

/**
 * \brief Retrieve the descriptor
 *
 * Get a Descriptor for the Module. The shared library has to implement
 * a method named getDescriptor with C linkage which returns a pointer
 * to a Descriptor object for this method to work.
 */
ModuleDescriptor	*Module::getDescriptor() {
	void	*s = getSymbol(std::string("getDescriptor"));

	// now cast the symbol to a function that returns a descriptor
	// pointer
	typedef ModuleDescriptor	*(*getter)();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "call the getDescriptor function");
	ModuleDescriptor	*d = ((getter)s)();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "module descriptor at %p", d);
	return d;
}

/**
 * \brief Retrieve the camera locator
 *
 * The camerae locator retrieved via this method can tell the list of
 * available cameras. The shared library has to implement a method
 * named getDeviceLocator with C linkage which returns a pointer to
 * a DeviceLocator object for this to work.
 */
DeviceLocatorPtr	Module::getDeviceLocator() {
	// lock to guarantee integrity of the devicelocator member
	std::lock_guard<std::recursive_mutex>	lock(_mutex);

	// if we have retrieved the device locator before, we can just
	// return it
	if (devicelocator) {
		return devicelocator;
	}

	// get the the device locator symbol
	void	*s = getSymbol(std::string("getDeviceLocator"));

	// now cast the symbol to a function that returns a descriptor pointer
	typedef DeviceLocator	*(*getter)();
	DeviceLocator	*c = ((getter)s)();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found device locator at %p", c);
	devicelocator = DeviceLocatorPtr(c);
	return devicelocator;
}

} // namespace module
} // namespace astro
