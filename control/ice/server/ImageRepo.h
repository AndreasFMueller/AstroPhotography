/*
 * ImageRepo.h -- auxiliary class to help locate the current image repo
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _ImageRepo_h
#define _ImageRepo_h

#include <AstroConfig.h>

namespace snowstar {

extern astro::config::ConfigurationKey	_snowstar_repositories_directory_key;

class ImageRepo {
public:
	ImageRepo() { }
	~ImageRepo() { }
	static astro::config::ConfigurationPtr	repoconfig();
	static astro::config::ImageRepoConfigurationPtr	imagerepoconfig();
	static astro::project::ImageRepoPtr	repo(const std::string& name);
	static std::string	configdb();
};

} // namespace snowstar

#endif /* _ImageRepo_h */
