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

uuidlist        RepositoryI::getUUIDs(const Ice::Current& /* current */) {
	uuidlist	result;
	std::set<astro::UUID>	uuids = _repo.getUUIDs("0 = 0");
	std::copy(uuids.begin(), uuids.end(), std::back_inserter(result));
	return result;
}

uuidlist	RepositoryI::getUUIDsCondition(const std::string& condition,
	const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve images with condition '%s'",
		condition.c_str());
	uuidlist	result;
	std::set<astro::UUID>	uuids = _repo.getUUIDs(condition);
	std::copy(uuids.begin(), uuids.end(), std::back_inserter(result));
	return result;
}

bool	RepositoryI::has(int id, const Ice::Current& /* current */) {
	return _repo.has(id);
}

bool	RepositoryI::hasUUID(const std::string& uuid,
		const Ice::Current& /* current */) {
	return _repo.has(astro::UUID(uuid));
}

int    RepositoryI::getId(const std::string& uuid,
			const Ice::Current& /* current */) {
	if (!_repo.has(astro::UUID(uuid))) {
		std::string	msg = astro::stringprintf("repo does not have "
			"%s", uuid.c_str());
		throw NotFound(msg);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get id of uuid %s", uuid.c_str());
	return _repo.getId(astro::UUID(uuid));
}

ImageFile       RepositoryI::getImage(int id,
		const Ice::Current& /* current */) {
	if (!_repo.has(id)) {
		std::string	msg = astro::stringprintf("repo does not have "
			"%d", id);
		throw NotFound(msg);
	}
	astro::image::ImagePtr	imageptr = _repo.getImage(id);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found image %d: %d x %d", id,
		imageptr->size().width(), imageptr->size().height());
	return convertfile(imageptr);
}

ImageInfo	RepositoryI::getInfo(int id,
			const Ice::Current& /* current */) {
	if (!_repo.has(id)) {
		std::string	msg = astro::stringprintf("repo does not have "
			"%d", id);
		throw NotFound(msg);
	}
	return convert(_repo.getEnvelope(id));
}

int    RepositoryI::save(const ImageFile& image,
		const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "request to save image of size %d",
		image.size());
	astro::image::ImagePtr	imageptr = convertfile(image);
	try {
		return _repo.save(imageptr);
	} catch (...) {
		throw Exists("Image already exists");
	}
}

void    RepositoryI::remove(int id, const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "request to remvoe %d", id);
	if (!_repo.has(id)) {
		std::string	msg = astro::stringprintf("repo does not have "
			"%d", id);
		throw NotFound(msg);
	}
	_repo.remove(id);
}

} // namespace snowstar
