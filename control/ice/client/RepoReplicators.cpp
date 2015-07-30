/*
 * RepoReplicators.cpp -- Implementation of image replicator classes
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <CommunicatorSingleton.h>
#include <getopt.h>
#include <AstroUtils.h>
#include <AstroDebug.h>
#include <AstroConfig.h>
#include <AstroFormat.h>
#include <repository.h>
#include <IceConversions.h>
#include <RepoReplicators.h>

using namespace snowstar;

namespace snowstar {
namespace app {
namespace snowrepo {

//////////////////////////////////////////////////////////////////////
// Implementation of BaseRepoReplicator
//////////////////////////////////////////////////////////////////////

std::set<std::string>	BaseRepoReplicator::getUUIDs(RepositoryPrx repo) {
	uuidlist u = repo->getUUIDsCondition(condition());
	std::set<std::string>	remoteuuids;
	std::copy(u.begin(), u.end(),
		std::inserter(remoteuuids, remoteuuids.begin()));
	return remoteuuids;
}


std::string	BaseRepoReplicator::condition() const {
	if (_project.size() > 0) {
		return astro::stringprintf("project = '%s'",
			_project.c_str());
	}
	return std::string("0 = 0");
}

BaseRepoReplicator::BaseRepoReplicator(const astro::URL& url,
	const std::string& project) : _project(project) {
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
	Ice::ObjectPrx	base = ic->stringToProxy(
					url.connect("Repositories"));
	remoterepositories = RepositoriesPrx::checkedCast(base);
	if (!remoterepositories) {
		throw std::runtime_error("no repositories proxy");
	}
	remoterepository = remoterepositories->get(url.path());
	if (!remoterepository) {
		throw std::runtime_error("no repository proxy");
	}
	remoteuuids = getUUIDs(remoterepository);
}

int	BaseRepoReplicator::push() {
	// get the uuids that should be copied
	std::list<std::string>	tocopy;
	std::set_difference(localuuids.begin(), localuuids.end(),
		remoteuuids.begin(), remoteuuids.end(),
		std::inserter(tocopy, tocopy.begin()));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d uuids to copy",
		tocopy.size());
	
	// get the ids for the images
	std::set<int>	copyids = getlocalids(tocopy);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d ids to copy",
		copyids.size());
	if (verbose()) {
		std::cout << "found " << copyids.size();
		std::cout << " files identified for push operation";
		std::cout << std::endl;
	}

	// now copy all the images to the remote repo
	for (auto ptr = copyids.begin(); ptr != copyids.end(); ptr++) {
		push(*ptr);
	}
	return EXIT_SUCCESS;
}

int	BaseRepoReplicator::pull() {
	// get the uuids that should be copied
	std::list<std::string>	tocopy;
	std::set_difference(remoteuuids.begin(), remoteuuids.end(),
		localuuids.begin(), localuuids.end(),
		std::inserter(tocopy, tocopy.begin()));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d uuids to copy",
		tocopy.size());
	
	// get the ids for the images
	std::set<int>	copyids;
	for (auto ptr = tocopy.begin(); ptr != tocopy.end(); ptr++) {
		copyids.insert(remoterepository->getId(*ptr));
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d ids to copy",
		copyids.size());
	if (verbose()) {
		std::cout << "found " << copyids.size();
		std::cout << " files identified for pull operation";
		std::cout << std::endl;
	}

	// copy the images from the remote repository
	for (auto ptr = copyids.begin(); ptr != copyids.end(); ptr++) {
		pull(*ptr);
	}
	return EXIT_SUCCESS;
}

int	BaseRepoReplicator::sync() {
	int	rc = push();
	if (rc) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "failed to push");
		return EXIT_FAILURE;
	}
	rc = pull();
	if (rc) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "failed to pull");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int	BaseRepoReplicator::command(const std::string& commandname) {
	if (commandname == "push") {
		return push();
	}
	if (commandname == "pull") {
		return pull();
	}
	if (commandname == "sync") {
		return sync();
	}
	std::cerr << "unknown command";
	return EXIT_FAILURE;
}


//////////////////////////////////////////////////////////////////////
// Implementation of LocalRepoReplicator
//////////////////////////////////////////////////////////////////////

std::set<std::string>	LocalRepoReplicator::getUUIDs(astro::project::ImageRepo& repo) {
	std::set<astro::UUID>	uuids = repo.getUUIDs(condition());
	std::set<std::string>	localuuids;
	std::copy(uuids.begin(), uuids.end(),
		std::inserter(localuuids, localuuids.begin()));
	return localuuids;
}

LocalRepoReplicator::LocalRepoReplicator(const std::string& localreponame,
	const astro::URL& remoteurl, const std::string& project)
	: BaseRepoReplicator(remoteurl, project) {
	astro::config::ConfigurationPtr	config
		= astro::config::Configuration::get();
	astro::config::ImageRepoConfigurationPtr	imagerepos
		= astro::config::ImageRepoConfiguration::get(config);
	localrepository = imagerepos->repo(localreponame);
	localuuids = getUUIDs(*localrepository);
}

std::set<int>	LocalRepoReplicator::getlocalids(const std::list<std::string>& tocopy) const {
	std::set<int>	copyids;
	for (auto ptr = tocopy.begin(); ptr != tocopy.end(); ptr++) {
		copyids.insert(localrepository->getId(astro::UUID(*ptr)));
	}
	return copyids;
}

void	LocalRepoReplicator::push(int id) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "copy image id %d", id);
	if (verbose()) {
		std::cout << "copy file " << id << std::endl;
	}
	if (!dryrun()) {
		astro::image::ImagePtr	imageptr
			= localrepository->getImage(id);
		remoterepository->save(convertfile(imageptr));
	}
}

void	LocalRepoReplicator::pull(int id) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "copy image id %d", id);
	if (verbose()) {
		std::cout << "pulling " << id << std::endl;
	}
	if (!dryrun()) {
		ImageFile imagefile = remoterepository->getImage(id);
		localrepository->save(convertfile(imagefile));
	}
}


//////////////////////////////////////////////////////////////////////
// Implementation of RemoteRepoReplicator
//////////////////////////////////////////////////////////////////////

RemoteRepoReplicator::RemoteRepoReplicator(const astro::URL& localurl,
	const astro::URL& remoteurl, const std::string& project)
	: BaseRepoReplicator(remoteurl, project) {
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
	Ice::ObjectPrx	base = ic->stringToProxy(
				localurl.connect("Repositories"));
	localrepositories = RepositoriesPrx::checkedCast(base);
	if (!localrepositories) {
		throw std::runtime_error("no repositories proxy");
	}
	localrepository = localrepositories->get(localurl.path());
	if (!remoterepository) {
		throw std::runtime_error("no repository proxy");
	}
	localuuids = getUUIDs(localrepository);
}

std::set<int>	RemoteRepoReplicator::getlocalids(const std::list<std::string>& tocopy) const {
	std::set<int>	copyids;
	for (auto ptr = tocopy.begin(); ptr != tocopy.end(); ptr++) {
		copyids.insert(localrepository->getId(*ptr));
	}
	return copyids;
}

void	RemoteRepoReplicator::push(int id) {	
	debug(LOG_DEBUG, DEBUG_LOG, 0, "copy image id %d", id);
	if (verbose()) {
		std::cout << "pushing " << id << std::endl;
	}
	if (!dryrun()) {
		ImageFile imagefile = localrepository->getImage(id);
		remoterepository->save(imagefile);
	}
}

void	RemoteRepoReplicator::pull(int id) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "copy image id %d", id);
	if (verbose()) {
		std::cout << "pulling " << id << std::endl;
	}
	if (!dryrun()) {
		ImageFile imagefile = remoterepository->getImage(id);
		localrepository->save(imagefile);
	}
}

} // namespace snowrepo
} // namespace app
} // namespace snowstar
