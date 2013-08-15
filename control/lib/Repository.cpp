/*
 * Repository.cpp -- repository class implementation
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroLoader.h>
#include <includes.h>
#include <iostream>

namespace astro {
namespace module {

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
void	Repository::checkpath(const std::string& path) const throw(repository_error) {
	// verify that the path exists and is a directory
	struct stat	sb;
	if (stat(path.c_str(), &sb) < 0) {
		std::string	message("cannot stat " + path + ": "
			+ strerror(errno));
		throw repository_error(message);
	}
	if (!S_ISDIR(sb.st_mode)) {
		throw repository_error(path + " is not a directory");
	}
}

/**
 * \brief Repository of modules contained in a directory.
 *
 * Create a Repository object representing the modules contained in the 
 * directory _path. The directory must already exist when the Repository
 * object is constructed (the directory is not created by the constructor)
 * and must be accessible by the user running the program.
 *
 * \param path		path to the directory containing the modules
 */
Repository::Repository(const std::string& path) throw (repository_error) : _path(path) {
	checkpath(_path);
}

/**
 * \brief Repository based on the pkglib directory.
 *
 * This is the prefered way to access a repository. Man applications
 * do not allow to specify a particular directory to search for
 * modules and thus rely on the modules being installed in the pkgdir
 * directory.
 */
Repository::Repository() throw (repository_error) : _path(PKGLIBDIR) {
	checkpath(_path);
}

/**
 * \brief Retrieve the number of modules available from the repository
 */
long	Repository::numberOfModules() const {
	return moduleNames().size();
}

/**
 * \brief Retrieve the module names
 *
 * This method just counts the module files that are installed, but it
 * may also count files that are ultimately not loadable
 */
std::vector<std::string>	Repository::moduleNames() const {
	std::vector<std::string>	result;
	// search the directory for files ending in la
	DIR	*dir = opendir(_path.c_str());
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
 * \brief Retrieve a list of all available modules in the Repository.
 *
 * The std::vector of ModulePtr objects returned is not just a list
 * of valid names. Each Module has already been checked whether the
 * file exists and is accessible.
 */
std::vector<ModulePtr>	Repository::modules() const {
	std::vector<ModulePtr>	result;
	// search the directory for files ending in la
	DIR	*dir = opendir(_path.c_str());
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
				result.push_back(ModulePtr(new Module(_path, modulename)));
			} catch (std::exception) {
				std::cerr << "module " << modulename <<
					" corrupt" << std::endl;
			}
		}
	}
	closedir(dir);
	return result;
}

/**
 * \brief Check whether a module is available in the repository.
 *
 * Modules are identified by their name. This function allows to
 * check whether a certain module is available in the Repository.
 * Since it actually instatiates (but does not return) the Module,
 * it also that not only the modules .la-file exists but rather that
 * that file is readable, contains a valid dlname specification and
 * the code file is also available and readable.
 */
bool	Repository::contains(const std::string& modulename) const {
	try {
		Module(_path, modulename);
	} catch (std::exception) {
		return false;
	}
	return true;
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
ModulePtr	Repository::getModule(const std::string& modulename) throw(repository_error) {
	try {
		if (modulecache.find(modulename) == modulecache.end()) {
			ModulePtr	module(new Module(_path, modulename));
			modulecache.insert(std::make_pair(modulename, module));
			return module;
		} else {
			return modulecache.find(modulename)->second;
		}
	} catch (std::exception& x) {
		throw repository_error(x.what());
	}
}

/**
 * \brief Retrieve the path of the repository.
 */
const std::string&      Repository::path() const { return _path; }

} // namespace module
} // namespace astro
