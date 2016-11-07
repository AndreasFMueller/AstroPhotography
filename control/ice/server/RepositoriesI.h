/*
 * RepositoresI.h
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Repositories_h
#define _Repositories_h

#include <repository.h>

namespace snowstar {

class RepositoriesI : virtual public Repositories {
public:
	RepositoriesI();
	virtual ~RepositoriesI();
	virtual reponamelist	list(const Ice::Current& current);
	virtual reposummarylist	summarylist(const Ice::Current& current);
	virtual bool	has(const std::string& reponame,
				const Ice::Current& current);
	virtual RepositoryPrx	get(const std::string& reponame,
					const Ice::Current& current);
	virtual void	add(const std::string& reponame,
				const std::string& directory,
				const Ice::Current& current);
	virtual void	remove(const std::string& reponame,
				bool removecontents,
				const Ice::Current& current);
	virtual bool	hidden(const std::string& reponame,
				const Ice::Current& current);
	virtual void	setHidden(const std::string& rerponame,
				bool hidden,
				const Ice::Current& current);
};

} // namespace snowstar

#endif /* _Repositories_h */
