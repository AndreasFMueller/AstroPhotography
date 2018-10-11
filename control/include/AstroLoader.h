/*
 * AstroLoader.h -- load shared libraries containing camera drivers (any maybe
 *             other types of objects)
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#ifndef _AstroLoader_h
#define _AstroLoader_h

#include <AstroCamera.h>
#include <AstroLocator.h>
#include <AstroExceptions.h>

#include <string>
#include <vector>
#include <stdexcept>
//#include <tr1/memory>
#include <memory>
#include <list>

namespace astro {
// the Module constructor is private, but we would like to test it
// individually, so we declare the ModuleTest class a friend
namespace test {
class ModuleTest;
}

namespace module {

/**
 * \brief Descriptor objects describe library modules.
 *
 * When a module is loaded, the application uses the getDescriptor
 * method of the Module class to get the Descriptor. It can then
 * use the Descriptor methods to learn more about the module.
 * In particular, it can find out what types of devices (cameras,
 * filter wheels, telescopes) are supported by the the module.
 */
class	ModuleDescriptor {
public:
	ModuleDescriptor() { }
	virtual	~ModuleDescriptor() { }
	virtual std::string	name() const;
	virtual std::string	version() const;
	virtual	bool	hasDeviceLocator() const;
};
typedef std::shared_ptr<ModuleDescriptor>	ModuleDescriptorPtr;

class ModuleRepository;
class ModuleRepositoryBackend;
/**
 * \brief Dynamically loadable module to control various types of
 *        devices used for astrophotography.
 *
 * A Module represents a dynamically loadable library used to control
 * devices used in astrophotography. Modules can represent driver 
 * libraries for cameras, filterwheels, telescopes or others.
 * Modules are usually returned by the modules or the getModule method
 * of the ModuleRepository class. Once loaded, some special functions in
 * the loadable library are used to return pointers to other objects.
 * E. g. every module must implement a function getDescriptor with C
 * linkage that returns a pointer to a Descriptor object. The getDescriptor
 * method of the Module class calls this function and wraps the pointer
 * into an ModuleDescriptorPtr smart pointer object. Other getter functions
 * are implemented similarly. See the member function documentation
 * for more information about methods that need to be implemented
 * by the library.
 */
class	Module {
	std::string	dirname;
	std::string	_modulename;
	std::string	dlname;
	void	*handle;	
	std::string	getDlname(const std::string& lafilename) const;
	bool	dlfileexists() const;
	Module(const std::string& dirname, const std::string& modulename);
	void	*getSymbol(const std::string& symbolname);
	astro::device::DeviceLocatorPtr	devicelocator;
public:
	bool	operator==(const Module& other) const;
	const std::string&	filename() const;
	const std::string&	modulename() const { return _modulename; }
	bool	isloaded() const { return NULL != handle; }
	void	open();
	void	close();
	static bool	dlclose_on_close;
	ModuleDescriptorPtr	getDescriptor();
	astro::device::DeviceLocatorPtr	getDeviceLocator();
	friend class ModuleRepositoryBackend;
	friend class ::astro::test::ModuleTest;
};

/**
 * \brief Smart pointer to Module objects
 *
 * Modules cannot be instantiated directly, and should not be copied,
 * because the Module class also has to keep track of the state whether
 * a module is currently linked into the address space. Therefore 
 * clients should only access the Module through a smart pointer
 * of type ModulePtr.
 */
typedef	std::shared_ptr<Module>	ModulePtr;

/**
 * \brief Exceptions thrown when the ModuleRepository class meets a problem.
 */
class	repository_error : public std::runtime_error {
public:
	repository_error(const std::string& whatString)
		: std::runtime_error(whatString) { }
};

typedef std::shared_ptr<ModuleRepository>	ModuleRepositoryPtr;

/**
 * \brief Class to retrieve module repositories
 */
ModuleRepositoryPtr	getModuleRepository();
ModuleRepositoryPtr	getModuleRepository(const std::string& path);

/**
 * \brief A ModuleRepository gives access to a collection of modules.
 *
 * A Repository object represents the loadable modules contained in
 * a given directory. The defualt constructor gives access to the
 * modules stored in the pkglib directory configured at configuration
 * time.
 *
 * The normal way to use the Repository class is to instantiate a single
 * instance of it, usually by using the default constructor. Then use
 * the modules method to find out about available modules. To instantiate
 * modules known by name, it is preferable to use the getModule method.
 */
class	ModuleRepository {
	std::string	_path;
public:
	ModuleRepository(const std::string& path) : _path(path) { }
	virtual ~ModuleRepository() { }
	const std::string&	path() const { return _path; }
	virtual long	numberOfModules() const = 0;
	virtual std::vector<std::string>	moduleNames() const = 0;
	virtual std::vector<ModulePtr>	modules() const = 0;
	virtual bool	contains(const std::string& modulename) = 0;
	virtual ModulePtr	getModule(const std::string& modulename) = 0;
};

/**
 * \brief The Devices object unifies access to devices across modules
 *
 * Going through the modules is usually not interesting for a user of
 * the devices, so we provide the Devices object to allow access to 
 * the devices directly.
 */
class Devices {
	ModuleRepositoryPtr	_repository;
public:
	Devices(ModuleRepositoryPtr repository) : _repository(repository) { }
	typedef	std::list<DeviceName>	devicelist;
	devicelist	getDevicelist(DeviceName::device_type type);
	camera::AdaptiveOpticsPtr	getAdaptiveOptics(const DeviceName& name);
	camera::CameraPtr	getCamera(const DeviceName& name);
	camera::CcdPtr		getCcd(const DeviceName& name);
	camera::GuidePortPtr	getGuidePort(const DeviceName& name);
	camera::FilterWheelPtr	getFilterWheel(const DeviceName& name);
	camera::CoolerPtr	getCooler(const DeviceName& name);
	camera::FocuserPtr	getFocuser(const DeviceName& name);
	device::MountPtr	getMount(const DeviceName& name);
};

} // namespace module
} // namespace astro

#endif /* _AstroLoader_h */
