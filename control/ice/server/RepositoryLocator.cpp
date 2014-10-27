/*
 * RepositoryLocator.cpp -- locator for repository servants
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <RepositoryI.h>
#include <RepositoryLocator.h>
#include <AstroDebug.h>
#include <AstroConfig.h>

namespace snowstar {

RepositoryLocator::RepositoryLocator() {
}

/** 
 * \brief Add a repository object to the locator map
 */
void	RepositoryLocator::add(const std::string& name,
		Ice::ObjectPtr repositoryptr) {
	if (repositories.find(name) != repositories.end()) {
		debug(LOG_WARNING, DEBUG_LOG, 0,
			"warning: repository '%s' already in map",
			name.c_str());
	}
	repositories.insert(std::make_pair(name, repositoryptr));
}

/**
 * \brief locate a repository in the map, create it if necessary
 */
Ice::ObjectPtr	RepositoryLocator::locate(const Ice::Current& current,
		Ice::LocalObjectPtr& /* cookie */) {
	// determine the repository name
	std::string	repositoryname = current.id.name;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "locate = %s", repositoryname.c_str());

	// check for the servant in the repository map
	repositorymap::iterator	i = repositories.find(repositoryname);
	if (i != repositories.end()) {
		return i->second;
	}

	// create the repository servant
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create a new repo servant");
	astro::config::ConfigurationPtr	config
		= astro::config::Configuration::get();
	Ice::ObjectPtr	repositoryptr = new RepositoryI(
		*config->repo(repositoryname));
	add(repositoryname, repositoryptr);
	return repositoryptr;
}

/**
 * \brief Stop using a servant
 */
void	RepositoryLocator::finished(const Ice::Current& /* current */,
		const Ice::ObjectPtr& /* servant */,
		const Ice::LocalObjectPtr& /* cookie */) {
}

/**
 * \brief deactivate servants
 */
void	RepositoryLocator::deactivate(const std::string& /* category */) {
}

} // namespace snowstar
