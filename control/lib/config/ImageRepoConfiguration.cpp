/*
 * ImageRepoConfiguration.cpp -- image repository configuration implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroConfig.h>
#include <AstroPersistence.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <cstdlib>
#include "ImageReposTable.h"
#include <includes.h>

using namespace astro::persistence;
using namespace astro::project;

namespace astro {
namespace config {

/**
 * \brief configuration backend
 *
 * This is used to hide the fact that there 
 */
class ImageRepoConfigurationBackend : public ImageRepoConfiguration {
	ConfigurationPtr	_config;
public:
	// constructor
	ImageRepoConfigurationBackend(ConfigurationPtr config)
		: _config(config) { }

	// access to repositories
	virtual bool	exists(const std::string& name);
	virtual ImageRepoPtr	repo(const std::string& name);
	virtual void	addrepo(const std::string& name,
				const std::string& directory);
	virtual void	removerepo(const std::string& name,
				bool removecontents);
	virtual std::list<ImageRepoInfo>	listrepo();
};

//////////////////////////////////////////////////////////////////////
// ImageRepoConfiguration implementation (static methods)
//////////////////////////////////////////////////////////////////////
ImageRepoConfigurationPtr	ImageRepoConfiguration::get() {
	return ImageRepoConfigurationPtr(
		new ImageRepoConfigurationBackend(Configuration::get()));
}

ImageRepoConfigurationPtr	ImageRepoConfiguration::get(
					ConfigurationPtr config) {
	return ImageRepoConfigurationPtr(
		new ImageRepoConfigurationBackend(config));
}

//////////////////////////////////////////////////////////////////////
// repository access
//////////////////////////////////////////////////////////////////////
/**
 * \brief Find out whether a repo exists
 */
bool	ImageRepoConfigurationBackend::exists(const std::string& name) {
	ImageRepoTable	repos(_config->database());
	return repos.contains(name);
}

/**
 * \brief get a repository
 */
ImageRepoPtr	ImageRepoConfigurationBackend::repo(const std::string& name) {
	ImageRepoTable	repos(_config->database());
	return ImageRepoPtr(new ImageRepo(repos.get(name)));
}

/**
 * \brief add a repository
 */
void	ImageRepoConfigurationBackend::addrepo(const std::string& name,
		const std::string& directory) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "add image repo %s in directory %s",
		name.c_str(), directory.c_str());

	// first find out whether the repository already exists
	if (exists(name)) {
		std::string	msg = stringprintf("image repository %s "
			"already exists", name.c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw BadParameter(msg);
	}

	// find out whether the name contains any slashes
	std::string	_directory = directory;
	if (directory.find('/') == std::string::npos) {
		if (!_config->hasglobal("repository", "topdir")) {
			debug(LOG_ERR, DEBUG_LOG, 0,
				"repository.topdir not set");
			throw BadParameter("repository.topdir not set");
		}
		_directory = _config->global("repository", "topdir")
			+ "/" + directory;
	}

	// find out whether the directory already exists
	struct stat	sb;
	if (0 == stat(_directory.c_str(), &sb)) {
		// is it a directory
		if (!(sb.st_mode & S_IFDIR)) {
			std::string	msg = stringprintf("%s is not a "
				"directory", _directory.c_str());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw BadParameter(msg);
		}

		// do we have access to the directory
		if (access(_directory.c_str(), R_OK | W_OK | X_OK) < 0) {
			std::string	msg = stringprintf("no access %s: %s",
				_directory.c_str(), strerror(errno));
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw BadParameter(msg);
		}
	} else {
		// create the directory
		if (mkdir(_directory.c_str(), 0777) < 0) {
			std::string	msg = stringprintf("cannot create "
				"directory %s: %s", _directory.c_str(),
				strerror(errno));
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw BadParameter(msg);
		}
	}

	// prepare the entry for the database
	ImageRepoRecord	imagerepoinfo;

	imagerepoinfo.reponame = name;
	imagerepoinfo.database = _directory + std::string("/.astro.db");
	imagerepoinfo.directory = _directory;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "using database name %s",
		imagerepoinfo.database.c_str());

	// add the repository info to the database
	ImageRepoTable	repos(_config->database());
	repos.add(imagerepoinfo);

	// find out whether the repository directory exists
	if (0 != stat(imagerepoinfo.database.c_str(), &sb)) {
		// file already exists, nothing needs to be done
		return;
	}

	// create a new repository
	Database	db = DatabaseFactory::get(imagerepoinfo.database);
	ImageRepo(name, db, _directory, false);
}

/**
 * \brief delete a repository
 */
void	ImageRepoConfigurationBackend::removerepo(const std::string& name,
		bool removecontents) {
	// first find out whether the name actuall exists
	if (!exists(name)) {
		std::string	msg = stringprintf("image repository '%s' does "
			"not exist", name.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw NotFound(msg);
	}

	// remove the contents
	if (removecontents) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "remove contents from repo %s",
			name.c_str());
		ImageRepoPtr	repoptr = repo(name);
		std::set<UUID>	uuids = repoptr->getUUIDs("0 = 0");
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d uuids", uuids.size());
		std::set<UUID>::const_iterator	i;
		for (i = uuids.begin(); i != uuids.end(); i++) {
			std::string	uuidstring = *i;
			debug(LOG_DEBUG, DEBUG_LOG, 0, "remove image %s", 
				uuidstring.c_str());
			repoptr->remove(*i);
		}
	}

	// remove the repository configuration from the database
	ImageRepoTable(_config->database()).remove(name);
}

/**
 * \brief Get a list of repositories in the configuration database
 */
std::list<ImageRepoInfo>	ImageRepoConfigurationBackend::listrepo() {
	ImageRepoTable	repos(_config->database());
	std::list<ImageRepoInfo>	result;
	std::list<ImageRepoRecord>	repolist = repos.select("0 = 0");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got %d image repo records",
		repolist.size());
	std::list<ImageRepoRecord>::const_iterator	i;
	for (i = repolist.begin(); i != repolist.end(); i++) {
		ImageRepoInfo	info;
		info.reponame = i->reponame;
		info.database = i->database;
		info.directory = i->directory;
		result.push_back(info);
	}
	return result;
}


} // namespace config
} // namespace astro
