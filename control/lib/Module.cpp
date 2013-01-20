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

namespace astro {
namespace module {

/**
 * \brief Control closing of dynamic library on module close 
 *
 * Under certain conditions, most notably when running CPPUNIT unit
 * tests, closing the dynamic library causes the program to crash.
 * The crash is caused by the DescriptorPtr returned by getDescriptor
 * being deallocated after the library has been closed. This can be
 * prevented if the either the returned DescriptorPtr goes out of
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
 */
std::string	Module::getDlname(const std::string& lafile) const {
	// open the file
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
			return dirname + "/" + std::string(p);
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
	if (stat(dlname.c_str(), &sb) < 0) {
		return false;
	}

	// check that the dlfile is accessible
	if (access(dlname.c_str(), R_OK) < 0) {
		return false;
	}

	// make sure the dl file is a file
	if (!S_ISREG(sb.st_mode)) {
		return false;
	}

	return true;
}

/**
 * \brief Construct a module given the repository directory and the module
 *        name.
 *
 * The file name of the .la file is constructed by concatenating the
 * directory name, the module name and the suffix .la.
 */
Module::Module(const std::string& _dirname, const std::string& _modulename)
	: dirname(_dirname), modulename(_modulename) {
	// get the path to the la file
	dlname = getDlname(dirname + "/" + modulename + ".la");

	if (!dlfileexists()) {
		throw std::domain_error("dl file not accessible");
	}
}

/**
 * \brief Returns the name of the code file (to be) loaded.
 */
const std::string&	Module::filename() const {
	return dlname;
}

/**
 * \brief Compare modules.
 */
bool	Module::operator==(const Module& other) const {
	return (other.dirname == dirname) && (other.modulename == modulename);
}

/**
 * \brief Open the module by loading and initializing it
 *
 * This method uses dlopen() to load the code file into the address space,
 * initializes the library and keeps a handle to the library for later
 * use. This method must be called before any module functions can be called.
 */
void	Module::open() {
	if (isloaded()) {
		return;
	}
	dlerror(); // clear error conditions
	handle = dlopen(dlname.c_str(), RTLD_NOW);
	if (NULL == handle) {
		std::string	error(dlerror());
		throw std::runtime_error("cannot load " + dlname + ": "
			+ error);
	}
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
	if ((handle) && (dlclose_on_close)) {
		dlclose(handle);
	}
	handle = NULL;
}

/**
 * \brief Retrieve the descriptor
 *
 * Get a Descriptor for the Module. The shared library has to implement
 * a method named getDescriptor with C linkage which returns a pointer
 * to a Descriptor object for this method to work.
 */
DescriptorPtr	Module::getDescriptor() const {
	if (!isloaded()) {
		throw std::runtime_error("module is not open");
	}
	// find the symbol for the getDescriptor function
	void	*s = dlsym(handle, "getDescriptor");
	if (NULL == s) {
		throw std::invalid_argument("getDescriptor not found");
	}

	// now cast the symbol to a function that returns a descriptor
	// pointer
	typedef Descriptor	*(*getter)();
	Descriptor	*d = ((getter)s)();
	return DescriptorPtr(d);
}

/**
 * \brief Retrieve the camera locator
 *
 * The camerae locator retrieved via this method can tell the list of
 * available cameras. The shared library has to implement a method
 * named getCameraLocator with C linkage which returns a pointer to
 * a CameraLocator object for this to work.
 */
astro::camera::CameraLocatorPtr	Module::getCameraLocator() const {
	if (!isloaded()) {
		throw std::runtime_error("module is not open");
	}
	// find the symbol for the getDescriptor function
	void	*s = dlsym(handle, "getCameraLocator");
	if (NULL == s) {
		throw std::invalid_argument("getCameraLocator not found");
	}

	// now cast the symbol to a function that returns a descriptor
	// pointer
	typedef astro::camera::CameraLocator	*(*getter)();
	astro::camera::CameraLocator	*c = ((getter)s)();
	return astro::camera::CameraLocatorPtr(c);
}

} // namespace module
} // namespace astro
