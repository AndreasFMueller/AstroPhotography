/*
 * RepositoryUser.h -- class implementing a repository in an interface
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _RepositoryUser_h
#define _RepositoryUser_h

#include <focusing.h>
#include <AstroConfig.h>

namespace snowstar {

class RepositoryUser {
	std::string	_repositoryname;
	astro::project::ImageRepoPtr	_imagerepo;
public:
	const std::string	repositoryname() const {
		return _repositoryname;
	}
	astro::project::ImageRepoPtr	imagerepo() {
		return _imagerepo;
	}
	// define the repository name to store images captured during
	// calibration or guiding
	virtual void	setRepositoryName(const std::string& reponame,
				const Ice::Current& current);
	virtual std::string	getRepositoryName(const Ice::Current& current);
};

} // namespace snowstar

#endif /* _RepositoryUser_h */
