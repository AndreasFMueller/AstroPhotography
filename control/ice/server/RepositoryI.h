/*
 * RepositoryI.h -- interface to repositories
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _RepositoryI_h
#define _RepositoryI_h

#include <repository.h>
#include <AstroConfig.h>
#include <AstroProject.h>

namespace snowstar {

class RepositoryI : virtual public Repository {
	astro::project::ImageRepo	_repo;
public:
	RepositoryI(astro::project::ImageRepo repo);
	virtual ~RepositoryI();
	virtual idlist	getIds(const Ice::Current& current);
	virtual idlist	getIdsCondition(const std::string& condition,
				const Ice::Current& current);
	virtual uuidlist	getUUIDs(const Ice::Current& current);
	virtual uuidlist	getUUIDsCondition(const std::string& condition,
					const Ice::Current& current);
	virtual projectnamelist	getProjectnames(const Ice::Current& current);
	virtual bool	has(int id, const Ice::Current& current);
	virtual bool	hasUUID(const std::string& uuid,
				const Ice::Current& current);
	virtual int	getId(const std::string& uuid,
				const Ice::Current& current);
	virtual ImageFile	getImage(int id, const Ice::Current& current);
	virtual ImageInfo	getInfo(int id, const Ice::Current& current);
	virtual int	save(const ImageFile& image,
				const Ice::Current& current);
	virtual int	count(const Ice::Current& current);
	virtual void	remove(int id, const Ice::Current& current);
};

} // namespace snowstar

#endif /* _RepositoryI_h */
