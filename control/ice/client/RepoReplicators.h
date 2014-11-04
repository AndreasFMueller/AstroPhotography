/*
 * RepoReplicators.h -- classes that perform image replication between different
 *                      types of repositories (local and remote)
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <CommunicatorSingleton.h>
#include <getopt.h>
#include <AstroUtils.h>
#include <AstroDebug.h>
#include <repository.h>

using namespace snowstar;

namespace snowstar {
namespace app {
namespace snowrepo {

/**
 * \brief base replicator class
 *
 * The base class takes care of the generic push/pull/sync operations, and
 * provides some methods to retrieve uuids from repsitories. Since there
 * is always at least one remote repository involved, we handle the remote
 * repository in the base class.
 */
class BaseRepoReplicator {
	bool	_verbose;
public:
	bool	verbose() const { return _verbose; }
	void	verbose(bool v) { _verbose = v; }
private:
	bool	_dryrun;
public:
	bool	dryrun() const { return _dryrun; }
	void	dryrun(bool d) { _dryrun = d; }
protected:
	RepositoriesPrx	remoterepositories;
	RepositoryPrx	remoterepository;
	std::set<std::string>	remoteuuids;
	std::set<std::string>	getUUIDs(RepositoryPrx repo);
	std::set<std::string>	localuuids;
	std::string	_project;
	std::string	condition() const;
public:
	BaseRepoReplicator(const astro::URL& url, const std::string& project);
	virtual std::set<int>	getlocalids(const std::list<std::string>& tocopy) const = 0;
	virtual void	push(const int id) = 0;
	virtual void	pull(const int id) = 0;
	int	push();
	int	pull();
	int	sync();
	int	command(const std::string& commandname);
};

/**
 * \brief Replicator class to replicate a local repository with a remote repo
 *
 * This class is used for replication between a local and a remote repository
 */
class LocalRepoReplicator : public BaseRepoReplicator {
protected:
	astro::project::ImageRepoPtr	localrepository;
	std::set<std::string>	getUUIDs(astro::project::ImageRepo& repo);
public:
	LocalRepoReplicator(const std::string& localreponame,
		const astro::URL& remoteurl, const std::string& project);
	virtual std::set<int>	getlocalids(const std::list<std::string>& tocopy) const;
	virtual void	push(int id);
	virtual void	pull(int id);
};

/**
 * \brief Replicator to replicate between two remote repositories
 *
 * The first remote repository is called the local repository although
 * it is remote image repository.
 */
class RemoteRepoReplicator : public BaseRepoReplicator {
protected:
	RepositoriesPrx	localrepositories;
	RepositoryPrx	localrepository;
public:
	RemoteRepoReplicator(const astro::URL& localurl,
		const astro::URL& remoteurl, const std::string& project);
	virtual std::set<int>	getlocalids(const std::list<std::string>& tocopy) const;
	virtual void	push(int id);
	virtual void	pull(int id);
};

} // namespace snowrepo
} // namespace app
} // namespace snowstar
