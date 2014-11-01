/*
 * RepoReplicator.cpp -- repository replicator class implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroProject.h>
#include <AstroDebug.h>
#include <algorithm>
#include <iterator>

namespace astro {
namespace project {

/** 
 * \brief Create a repository replicator
 */
RepoReplicator::RepoReplicator() {
}

std::set<long>	RepoReplicator::uuid2ids(ImageRepoPtr repo,
		const std::set<UUID>& uuids) {
	std::set<long>	result;
	for (auto ptr = uuids.begin(); ptr != uuids.end(); ptr++) {
		result.insert(repo->getId(*ptr));
	}
	return result;
}

/**
 * \brief Replicate images from one repository to another
 *
 * \param remove	If this flag is set then images that are not in the
 *			source directory are deleted from target directory
 */
int	RepoReplicator::replicate(ImageRepoPtr src, ImageRepoPtr dst,
	bool remove) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "replicating from %s to %s",
		src->name().c_str(), dst->name().c_str());
	int	count = 0;
	std::set<UUID>	srcuuids = src->getUUIDs(std::string("0 = 0"));
	std::set<UUID>	dstuuids = dst->getUUIDs(std::string("0 = 0"));
	std::set<UUID>	tocopy;

	// determine the difference
	std::set_difference(
		srcuuids.begin(), srcuuids.end(),
		dstuuids.begin(), dstuuids.end(),
		std::inserter(tocopy, tocopy.begin()));

	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d items to replicate",
		tocopy.size());
	std::set<long>	ids = uuid2ids(src, tocopy);

	// copy the entries
	for (auto ptr = ids.begin(); ptr != ids.end(); ptr++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"copy id %d to repo %s", *ptr, dst->name().c_str());
		dst->save(src->getImage(*ptr));
		count++;
	}

	// if the remove flag is set, remove the 
	if (!remove) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "don't delete");
		return count;
	}

	// determine what UUIDs are only present in the target, so they
	// should be removed
	debug(LOG_DEBUG, DEBUG_LOG, 0, "remove deleted images");
	std::set<UUID>	toremove;
	int	removed = 0;
	std::set_difference(
		dstuuids.begin(), dstuuids.end(),
		srcuuids.begin(), srcuuids.end(),
		std::inserter(toremove, toremove.begin()));
	ids = uuid2ids(dst, toremove);
	for (auto ptr = ids.begin(); ptr != ids.end(); ptr++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "remove %d from %s",
			*ptr, dst->name().c_str());
		dst->remove(*ptr);
		removed++;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "images removed: %d", removed);

	// done, return number of images copied
	return count;
}

/**
 * \brief synchronize images between two repositories
 *
 * This methods ensures that all images are present in both repositories
 */
int	RepoReplicator::synchronize(ImageRepoPtr repo1, ImageRepoPtr repo2) {
	return replicate(repo1, repo2, false) + replicate(repo2, repo1, false);
}

} // namespace project
} // namespacce astro
