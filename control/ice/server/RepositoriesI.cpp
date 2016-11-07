/*
 * RepositoriesI.cpp
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <RepositoriesI.h>
#include <ProxyCreator.h>
#include <AstroConfig.h>
#include <AstroFormat.h>

namespace snowstar {

RepositoriesI::RepositoriesI() {
}

RepositoriesI::~RepositoriesI() {
}

/**
 * \brief Retrieve a list of repository names known to the configuration
 */
reponamelist	RepositoriesI::list(const Ice::Current& /* current */) {
	reponamelist	result;
	// retrieve a list of repository names from the configuration
	astro::config::ConfigurationPtr	config
		= astro::config::Configuration::get();
	astro::config::ImageRepoConfigurationPtr	imagerepos
		= astro::config::ImageRepoConfiguration::get(config);
	std::list<astro::project::ImageRepoInfo>	repolist
		= imagerepos->listrepo(true);
	for (auto ptr = repolist.begin(); ptr != repolist.end(); ptr++) {
		result.push_back(ptr->reponame);
	}
	return result;
}

/**
 * \brief Retrieve a list of repository summaries of all repositories
 */
reposummarylist	RepositoriesI::summarylist(const Ice::Current& /* current */) {
	reposummarylist	result;
	// retrieve a list of repository names from the configuration
	astro::config::ConfigurationPtr	config
		= astro::config::Configuration::get();
	astro::config::ImageRepoConfigurationPtr	imagerepos
		= astro::config::ImageRepoConfiguration::get(config);
	std::list<astro::project::ImageRepoInfo>	repolist
		= imagerepos->listrepo(false);
	for (auto ptr = repolist.begin(); ptr != repolist.end(); ptr++) {
		RepositorySummary	summary;
		summary.name = ptr->reponame;
		summary.directory = ptr->directory;
		summary.database = ptr->database;
		summary.hidden = ptr->hidden;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "repo %s: %s -> %s",
			summary.name.c_str(),
			(ptr->hidden) ? "hidden" : "visible",
			(summary.hidden) ? "hidden" : "visible");
		astro::config::ConfigurationPtr configuration
			= astro::config::Configuration::get();
		astro::project::ImageRepo	repo(ptr->reponame,
			configuration->database(), ptr->directory, false);
		summary.count = repo.count();
		result.push_back(summary);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d repository records",
		result.size());
	return result;
}

/**
 * \brief Find out whether an image repository of a given name exists
 */
bool	RepositoriesI::has(const std::string& reponame,
			const Ice::Current& /* current */) {
	// retrieve a list of repository names from the configuration
	astro::config::ConfigurationPtr	config
		= astro::config::Configuration::get();
	astro::config::ImageRepoConfigurationPtr	imagerepos
		= astro::config::ImageRepoConfiguration::get(config);
	return imagerepos->exists(reponame);
}

/**
 * \brief Retrieve a proxy to an image repository
 */
RepositoryPrx	RepositoriesI::get(const std::string& reponame,
			const Ice::Current& current) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "request repo '%s'", reponame.c_str());
	return createProxy<RepositoryPrx>("repository/" + reponame, current,
		false);
}

/**
 * \brief remove a image repository from the configuration
 *
 * This method removes an image repository from the configuration database
 * and removes its content if the removecontents parameter is true.
 */
void	RepositoriesI::remove(const std::string& reponame, bool removecontents,
		const Ice::Current& /* current */) {
	// get configuration
	astro::config::ConfigurationPtr	config
		= astro::config::Configuration::get();
	astro::config::ImageRepoConfigurationPtr	imagerepos
		= astro::config::ImageRepoConfiguration::get(config);

	// check repo exists
	if (!imagerepos->exists(reponame)) {
		std::string	msg = astro::stringprintf("image repository "
			"'%s' does not exist", reponame.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw NotFound(msg);
	}
	
	// remove the repository
	try {
		imagerepos->removerepo(reponame, removecontents);
	} catch (const std::exception& x) {
		std::string	msg = astro::stringprintf("cannot remove "
			"repository: %s", x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw IOException(msg);
	}
}

/**
 * \brief add a repository
 *
 * This method creates an image repository definition in the local
 * configuration database, adding the images found within. It throws an
 * exception if the repository cannot be generated.
 */
void	RepositoriesI::add(const std::string& reponame,
		const std::string& directory,
		const Ice::Current& /* current */) {
	// get configuration
	astro::config::ConfigurationPtr	config
		= astro::config::Configuration::get();
	astro::config::ImageRepoConfigurationPtr	imagerepos
		= astro::config::ImageRepoConfiguration::get(config);

	// check whether repository already exists
	if (imagerepos->exists(reponame)) {
		std::string	msg = astro::stringprintf("image repository "
			"'%s' already exists", reponame.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw Exists(msg);
	}

	// create the repository
	try {
		imagerepos->addrepo(reponame, directory);
	} catch (const std::runtime_error& x) {
		std::string	msg = astro::stringprintf("cannot create image "
			"repository '%s' in directory %s: %s",
			reponame.c_str(), directory.c_str(), x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw BadParameter(msg);
	}
}

/**
 * \brief return hidden status of a repository
 */
bool	RepositoriesI::hidden(const std::string& reponame,
		const Ice::Current& /* current */) {
	// get configuration
	astro::config::ConfigurationPtr	config
		= astro::config::Configuration::get();
	astro::config::ImageRepoConfigurationPtr	imagerepos
		= astro::config::ImageRepoConfiguration::get(config);

	// check whether repository already exists
	if (imagerepos->exists(reponame)) {
		std::string	msg = astro::stringprintf("image repository "
			"'%s' does not exist", reponame.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw NotFound(msg);
	}
	return imagerepos->hidden(reponame);
}

/**
 * \brief Hide/unhide a repository
 */
void	RepositoriesI::setHidden(const std::string& reponame,
		bool hidden, const Ice::Current& /* current */) {
	// get configuration
	astro::config::ConfigurationPtr	config
		= astro::config::Configuration::get();
	astro::config::ImageRepoConfigurationPtr	imagerepos
		= astro::config::ImageRepoConfiguration::get(config);

	// check whether repository already exists
	if (!imagerepos->exists(reponame)) {
		std::string	msg = astro::stringprintf("image repository "
			"'%s' does not exist", reponame.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw NotFound(msg);
	}

	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "setting repo '%s' to %s",
			reponame.c_str(),
			(hidden) ? "hidden" : "visible");
		imagerepos->setHidden(reponame, hidden);
	} catch (const std::exception& x) {
		std::string	msg = astro::stringprintf(
			"cannot set hidden: %s", x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw BadParameter(msg);
	}
}

} // namespace snowstar
