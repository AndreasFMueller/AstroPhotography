/*
 * RepositoryUser.cpp -- Implementation of RepositoryUser class
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "RepositoryUser.h"

namespace snowstar {

RepositoryUser::RepositoryUser() {
}

RepositoryUser::~RepositoryUser() {
}

RepositoryUser::RepositoryUser(const std::string& reponame) {
	repositoryname(reponame);
}

/**
 * \brief retrieve the name of the current repository
 */
std::string	RepositoryUser::getRepositoryName(
			const Ice::Current& /* current */) {
	return _repositoryname;
}

/**
 * \brief activate sending images to the repository
 */
void	RepositoryUser::setRepositoryName(const std::string& reponame,
		const Ice::Current& /* current */) {
	repositoryname(reponame);
}

void	RepositoryUser::repositoryname(const std::string& reponame) {
	// special case: zero length repo name means turn of storing images
	// in the repository
	if (0 == reponame.size()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "removing repository '%s'",
			_repositoryname.c_str());
		_repositoryname = reponame;
		_imagerepo.reset();
		return;
	}

	// check that this repository actually exists
	astro::config::ImageRepoConfigurationPtr	config
		= astro::config::ImageRepoConfiguration::get();
	if (!config->exists(reponame)) {
		// throw an error
		NotFound	exception;
		exception.cause = astro::stringprintf("repository %s not found",
			reponame.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", exception.cause.c_str());
		throw exception;
	}
	_imagerepo = config->repo(reponame);
	_repositoryname = reponame;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "using repository %s",
		_repositoryname.c_str());
}

} // namespace snowstar
