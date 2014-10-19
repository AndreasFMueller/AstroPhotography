/*
 * RepositoresI.h
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Repositories_h
#define _Repositories_h

#include <repository.h>

namespace snowstar {

class RepositoriesI : public Repositories {
public:
	RepositoriesI();
	virtual ~RepositoriesI();
	virtual reponamelist	list(const Ice::Current& current);
	virtual RepositoryPrx	get(const std::string& reponame,
					const Ice::Current& current);
};

} // namespace snowstar

#endif /* _Repositories_h */
