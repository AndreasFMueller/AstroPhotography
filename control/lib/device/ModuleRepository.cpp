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
#include <fstream>

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
	ModuleRepositories(const ModuleRepositories&) = delete;
	ModuleRepositories&	operator=(const ModuleRepositories&) = delete;
public:
	ModuleRepositories();
	ModuleRepositoryPtr	get(const std::string& path);
};

static ModuleRepositories	repositories;

ModuleRepositories::ModuleRepositories() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create a module repository at %p",
		this);
}

ModuleRepositoryPtr	ModuleRepository::get() {
	return repositories.get(PKGLIBDIR);
}

ModuleRepositoryPtr	ModuleRepository::get(const std::string& path) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve module repo at %s",
		path.c_str());
	return repositories.get(path);
}

//////////////////////////////////////////////////////////////////////
// ModuleRepository
//////////////////////////////////////////////////////////////////////
ModuleRepository::ModuleRepository(const std::string& path) : _path(path) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "constructing module repository at %s",
		path.c_str());
}

//////////////////////////////////////////////////////////////////////
// ModuleRepositoryBackend
//////////////////////////////////////////////////////////////////////
/**
 * \brief Repository backend class
 *
 * The repository backend is what the Repositories class returns.
 *
 * This backend implementation also includes a blacklisting mechanism.
 * A file named 'blacklist' with names of modules to ignore prevents
 * these modules from being loaded. The file can contain comment
 * lines starting with #.
 */
class ModuleRepositoryBackend : public ModuleRepository {
private:
	std::recursive_mutex	_mutex;	// protect the map
	typedef	std::map<std::string, ModulePtr>	modulemap;
	std::list<std::string>	blacklisted;
	modulemap	modulecache;
	void	checkpath(const std::string& path) const;
	void	readblacklist();
	bool	isblacklisted(const std::string& name) const;
	ModuleRepositoryBackend(const ModuleRepositoryBackend&) = delete;
	ModuleRepositoryBackend&	operator=(
		const ModuleRepositoryBackend&) = delete;
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
 *
 * \param path		path where the modules are located
 */
ModuleRepositoryPtr	ModuleRepositories::get(const std::string& path) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve backend for directory '%s'",
		path.c_str());
	std::string	key = path;
	if (key.size() == 0) {
		key = std::string(PKGLIBDIR);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "key for empty path is %s",
			key.c_str());
	}

	// make sure we are the only thread working on the backend
	std::unique_lock<std::recursive_mutex>	lock(mutex);

	// find the backend
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
	readblacklist();
}

/**
 * \brief Read the blacklist file
 */
void	ModuleRepositoryBackend::readblacklist() {
	// build the path the the blacklist file
	std::string	blacklistfile = path() + std::string("/blacklist");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "path to blacklist: '%s'",
		blacklistfile.c_str());

	// find out whether the blacklist file exists
	try {
		std::ifstream	in(blacklistfile.c_str());
		if (in.bad()) {
			debug(LOG_ERR, DEBUG_LOG, 0, "could not open '%s'",
				blacklistfile.c_str());
		}

		// read the blacklist file
		while (in.good() && !in.eof()) {
			char	b[1024];
			in.getline(b, sizeof(b));
			std::string	modulename = trim(std::string(b));
			// check for empty lines
			if (modulename.size() == 0) {
				continue;
			}
			// check for comments
			if (modulename[0] == '#') {
				continue;
			}
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"blacklisted module: '%s'", modulename.c_str());
			blacklisted.push_back(modulename);
		}
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "could not parse %s: %s",
			blacklistfile.c_str(), x.what());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "blacklist file read");
}

/**
 * \brief Find out whether a module is blacklisted
 *
 * \param modulename	name of the module to check
 */
bool	ModuleRepositoryBackend::isblacklisted(const std::string& modulename) const {
	for (auto a : blacklisted) {
		if (a == modulename)
			return true;
	}
	return false;
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
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot open module dir %s: %s",
			path().c_str(), strerror(errno));
		return result;
	}
	struct dirent	*direntp = NULL;
	do {
		direntp = readdir(dir);
		if (NULL == direntp)
			continue;
		int	namelen = strlen(direntp->d_name);
		if (0 == strcmp(".la", direntp->d_name + namelen - 3)) {
			std::string	modulename
				= std::string(direntp->d_name).substr(0,
					namelen - 3);
			if (isblacklisted(modulename))
				continue;
			result.push_back(modulename);
		}
	} while (direntp != NULL);
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
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot open module dir %s: %s",
			path().c_str(), strerror(errno));
		return result;
	}
	struct dirent	*direntp = NULL;
	do {
		direntp = readdir(dir);
		if (NULL == direntp)
			continue;
		int	namelen = strlen(direntp->d_name);
		if (0 == strcmp(".la", direntp->d_name + namelen - 3)) {
			std::string	modulename
				= std::string(direntp->d_name).substr(0,
					namelen - 3);
			// skip if module is blacklisted
			if (isblacklisted(modulename))
				continue;
			try {
				result.push_back(ModulePtr(new Module(path(),
					modulename)));
			} catch (const std::exception& x) {
				std::cerr << "module " << modulename <<
					" corrupt: ";
				std::cerr << x.what() << std::endl;
			}
		}
	} while (direntp != NULL);
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

	// if the module is blacklisted, give up
	if (isblacklisted(modulename)) {
		return false;
	}

	// first find out whether we have already checked this module
	if (modulecache.find(modulename) != modulecache.end()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "module '%s' already loaded",
			modulename.c_str());
		return true;
	}

	// if now, try to find the module and insert it into the module cache
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "try to load '%s'",
			modulename.c_str());
		ModulePtr	module(new Module(path(), modulename));
		modulecache.insert(std::make_pair(modulename, module));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "module '%s' created at %p",
			modulename.c_str(), &*module);
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
	// make sure a blacklisted modules are not requested
	if (isblacklisted(modulename)) {
		std::string	msg = stringprintf("module '%s' blacklisted",
			modulename.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw repository_error(msg);
	}

	// now get the module
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get module '%s'", modulename.c_str());
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	if (contains(modulename)) {
		// if we get to this point, the module already has been loaded
		// so we can just return the module
		return modulecache.find(modulename)->second;
	}
	// at this point, the module does not exists and we throw an exception
	std::string	msg = stringprintf("module %s not found",
				modulename.c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw repository_error(msg);
}

} // namespace module
} // namespace astro
