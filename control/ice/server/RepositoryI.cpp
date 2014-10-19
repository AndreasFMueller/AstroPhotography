/*
 * RepositoryI.cpp -- Repository servant implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <RepositoryI.h>
#include <IceConversions.h>
#include <AstroDebug.h>

namespace snowstar {

RepositoryI::RepositoryI(astro::project::ImageRepo repo) : _repo(repo) {
}

RepositoryI::~RepositoryI() {
}

uuidlist        RepositoryI::getUUIDs(const Ice::Current& current) {
	uuidlist	result;
	std::set<astro::UUID>	uuids = _repo.getUUIDs("0 = 0");
	std::copy(uuids.begin(), uuids.end(), std::back_inserter(result));
	return result;
}

int    RepositoryI::getId(const std::string& uuid,
			const Ice::Current& current) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get id of uuid %s", uuid.c_str());
	return _repo.getId(astro::UUID(uuid));
}

ImageFile       RepositoryI::getImage(int id, const Ice::Current& current) {
	astro::image::ImagePtr	imageptr = _repo.getImage(id);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found image %d: %d x %d", id,
		imageptr->size().width(), imageptr->size().height());
	return convertfile(imageptr);
}

int    RepositoryI::save(const ImageFile& image,
		const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "request to save image of size %d",
		image.size());
	astro::image::ImagePtr	imageptr = convertfile(image);
	return _repo.save(imageptr);
}

void    RepositoryI::remove(int id, const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "request to remvoe %d", id);
	_repo.remove(id);
}

} // namespace snowstar
