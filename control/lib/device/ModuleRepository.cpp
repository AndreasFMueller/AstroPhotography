/*
 * ModuleRepository.cpp -- module repository class implementation
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroLoader.h>
#include <includes.h>
#include <iostream>
#include <AstroUtils.h>
#include <AstroDebug.h>
#include <mutex>

namespace astro {
namespace module {

//////////////////////////////////////////////////////////////////////
// Repositories collection
//////////////////////////////////////////////////////////////////////
/**
 * \brief A collection of RepositoryBackends
 *
 * A static object of this type gives access to all backends that have been
 * accessed by a program. There may be multiple directories containing
 * driver modules, and we don't want to open them over and over again.
 * The Repositories object mediates access to the repositories and thus
 * ensures that each repository is instantiated only once.
 */
class ModuleRepositories {
	std::recursive_mutex	mutex;
	typedef	std::map<std::string, ModuleRepositoryPtr>	backendmap;
	backendmap	_repositories;
public:
	ModuleRepositories();
	ModuleRepositoryPtr	get(const std::string& path);
};

static ModuleRepositories	repositories;

ModuleRepositories::ModuleRepositories() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create a module repository at %p",
		this);
}

ModuleRepositoryPtr	getModuleRepository() {
	return repositories.get(PKGLIBDIR);
}

ModuleRepositoryPtr	getModuleRepository(const std::string& path) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve module repo at %s",
		path.c_str());
	return repositories.get(path);
}

//////////////////////////////////////////////////////////////////////
// Repositories collection
//////////////////////////////////////////////////////////////////////
/**
 * \brief Repository backend class
 *
 * The repository backend is what the Repositories class returns.
 */
class ModuleRepositoryBackend : public ModuleRepository {
private:
	std::recursive_mutex	_mutex;	// protect the map
	typedef	std::map<std::string, ModulePtr>	modulemap;
	modulemap	modulecache;
	void	checkpath(const std::string& path) const;
public:
	ModuleRepositoryBackend(const std::string& path);
	virtual ~ModuleRepositoryBackend() { }
	virtual long	numberOfModules() const;
	virtual std::vector<std::string>	moduleNames() const;
	virtual std::vector<ModulePtr>	modules() const;
	virtual bool	contains(const std::string& modulename);
	virtual ModulePtr	getModule(const std::string& modulename);
};

/**
 * \brief Retrieve a repository backend associated with a path
 */
ModuleRepositoryPtr	ModuleRepositories::get(const std::string& path) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve backend for '%s'",
		path.c_str());
	std::string	key = path;
	if (key.size() == 0) {
		key = std::string(PKGLIBDIR);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "key for empty path is %s",
			key.c_str());
	}
	std::unique_lock<std::recursive_mutex>	lock(mutex);
	backendmap::iterator	r = _repositories.find(key);
	if (r != _repositories.end()) {
		return r->second;
	}

	// there is no backend yet, so we have to create it
	ModuleRepositoryPtr	rbp(new ModuleRepositoryBackend(key));
	_repositories.insert(std::make_pair(key, rbp));
	return rbp;
}

/**
 * \brief Auxiliary function used to check accessibility of a repository
 *        path
 *
 * the checkpath function verifies that the path exists and is actually
 * a directory. It is used by the constructors.
 *
 * \param path	directory path to verify.
 * \throws repository_error	is thrown when there is any problem with the
 *				directory specified
 */
void	ModuleRepositoryBackend::checkpath(const std::string& path) const {
	// verify that the path exists and is a directory
	struct stat	sb;
	if (stat(path.c_str(), &sb) < 0) {
		std::string	message("cannot stat '" + path + "': "
			+ strerror(errno));
		throw repository_error(message);
	}
	if (!S_ISDIR(sb.st_mode)) {
		throw repository_error(path + " is not a directory");
	}
}

/**
 * \brief ModuleRepository of modules contained in a directory.
 *
 * Create a ModuleRepository object representing the modules contained in the 
 * directory _path. The directory must already exist when the ModuleRepository
 * object is constructed (the directory is not created by the constructor)
 * and must be accessible by the user running the program.
 *
 * \param path		path to the directory containing the modules
 */
ModuleRepositoryBackend::ModuleRepositoryBackend(const std::string& p)
	: ModuleRepository(p) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating repository backend at %s",
		path().c_str());
	checkpath(path());
}

/**
 * \brief Retrieve the number of modules available from the repository
 */
long	ModuleRepositoryBackend::numberOfModules() const {
	return moduleNames().size();
}

/**
 * \brief Retrieve the module names
 *
 * This method just counts the module files that are installed, but it
 * may also count files that are ultimately not loadable
 */
std::vector<std::string>	ModuleRepositoryBackend::moduleNames() const {
	std::vector<std::string>	result;
	// search the directory for files ending in la
	DIR	*dir = opendir(path().c_str());
	if (NULL == dir) {
		return result;
	}
	struct dirent	*dirent;
	while (NULL != (dirent = readdir(dir))) {
		int	namelen = strlen(dirent->d_name);
		if (0 == strcmp(".la", dirent->d_name + namelen - 3)) {
			std::string	modulename
				= std::string(dirent->d_name).substr(0, namelen - 3);
			result.push_back(modulename);
		}
	}
	closedir(dir);
	return result;
	
}

/**
 * \brief Retrieve a list of all available modules in the ModuleRepository.
 *
 * The std::vector of ModulePtr objects returned is not just a list
 * of valid names. Each Module has already been checked whether the
 * file exists and is accessible.
 */
std::vector<ModulePtr>	ModuleRepositoryBackend::modules() const {
	std::vector<ModulePtr>	result;
	// search the directory for files ending in la
	DIR	*dir = opendir(path().c_str());
	if (NULL == dir) {
		return result;
	}
	struct dirent	*dirent;
	while (NULL != (dirent = readdir(dir))) {
		int	namelen = strlen(dirent->d_name);
		if (0 == strcmp(".la", dirent->d_name + namelen - 3)) {
			std::string	modulename
				= std::string(dirent->d_name).substr(0, namelen - 3);
			try {
				result.push_back(ModulePtr(new Module(path(), modulename)));
			} catch (std::exception) {
				std::cerr << "module " << modulename <<
					" corrupt" << std::endl;
			}
		}
	}
	closedir(dir);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d modules", result.size());
	return result;
}

/**
 * \brief Check whether a module is available in the repository.
 *
 * Modules are identified by their name. This function allows to
 * check whether a certain module is available in the ModuleRepository.
 * Since it actually instatiates (but does not return) the Module,
 * it also that not only the modules .la-file exists but rather that
 * that file is readable, contains a valid dlname specification and
 * the code file is also available and readable.
 */
bool	ModuleRepositoryBackend::contains(const std::string& modulename) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "check for module %s",
		modulename.c_str());
	std::unique_lock<std::recursive_mutex>	lock(_mutex);

	// first find out whether we have already checked this module
	if (modulecache.find(modulename) != modulecache.end()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "module '%s' loaded",
			modulename.c_str());
		return true;
	}

	// if now, try to find the module and insert it into the module cache
	try {
		ModulePtr	module(new Module(path(), modulename));
		modulecache.insert(std::make_pair(modulename, module));
		return true;
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot load '%s': %s",
			modulename.c_str(), x.what());
	}
	return false;
}

/**
 * \brief Get a module by name.
 *
 * Retrieve a module by name. An repository_error exception is thrown if
 * the Module cannot be found or instanciated.
 * \throws runtime_error	Indicates that the module could not found,
 *				has a corrupt .la file or the code file
 *				is missing.
 */
ModulePtr	ModuleRepositoryBackend::getModule(const std::string& modulename) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get module '%s'", modulename.c_str());
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	if (contains(modulename)) {
		return modulecache.find(modulename)->second;
	}
	std::string	msg = stringprintf("module %s not found",
				modulename.c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw repository_error(msg);
}

} // namespace module
} // namespace astro