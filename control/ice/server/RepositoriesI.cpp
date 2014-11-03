/*
 * RepositoriesI.cpp
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <RepositoriesI.h>
#include <ProxyCreator.h>
#include <AstroConfig.h>

namespace snowstar {

RepositoriesI::RepositoriesI() {
}

RepositoriesI::~RepositoriesI() {
}

reponamelist	RepositoriesI::list(const Ice::Current& /* current */) {
	reponamelist	result;
	// retrieve a list of repository names from the configuration
	astro::config::ConfigurationPtr	config
		= astro::config::Configuration::get();
	std::list<astro::project::ImageRepoInfo>	repolist
		= config->listrepo();
	for (auto ptr = repolist.begin(); ptr != repolist.end(); ptr++) {
		result.push_back(ptr->reponame);
	}
	return result;
}

RepositoryPrx	RepositoriesI::get(const std::string& reponame,
			const Ice::Current& current) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "request repo '%s'", reponame.c_str());
	return createProxy<RepositoryPrx>("repository/" + reponame, current,
		false);
}

} // namespace snowstar
