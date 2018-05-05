/*
 * ImageRepo.cpp -- auxiliary class to help locate the current image repo
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <ImageRepo.h>

namespace snowstar {

/**
 * \brief Get the database file name
 */
std::string	ImageRepo::configdb() {
	astro::config::ConfigurationPtr config
		= astro::config::Configuration::get();
	if (config->has("snowstar", "repositories", "directory")) {
		return config->get("snowstar", "repositories", "directory");
	}
	return std::string("");
}

/**
 * \brief Get the current image repo configuration database
 */
astro::config::ConfigurationPtr	ImageRepo::repoconfig() {
	// get the repository database name
	astro::config::ConfigurationPtr config
		= astro::config::Configuration::get();
	if (config->has("snowstar", "repositories", "directory")) {
		std::string	repodbname
			= config->get("snowstar", "repositories", "directory");
		config = astro::config::Configuration::get(repodbname);
	}
	return config;
}

/**
 * \brief Retrieve the image repository for a reponame
 */
astro::config::ImageRepoConfigurationPtr	ImageRepo::imagerepoconfig() {
	astro::config::ConfigurationPtr config = repoconfig();
        astro::config::ImageRepoConfigurationPtr        imagerepos
                = astro::config::ImageRepoConfiguration::get(config);
	return imagerepos;
}

/**
 * \brief Retrieve an image repository
 */
astro::project::ImageRepoPtr	ImageRepo::repo(const std::string& reponame) {
	astro::config::ImageRepoConfigurationPtr imagerepos = imagerepoconfig();
	astro::project::ImageRepoPtr	repo = imagerepos->repo(reponame);
	return repo;
}

} // namespace snowstar
