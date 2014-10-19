/*
 * RepositoryI.cpp -- Repository servant implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <RepositoryI.h>
#include <IceConversions.h>

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
	return _repo.getId(astro::UUID(uuid));
}

ImageFile       RepositoryI::getImage(int id, const Ice::Current& current) {
	return convertfile(_repo.getImage(id));
}

int    RepositoryI::save(const ImageFile& image,
		const Ice::Current& /* current */) {
	astro::image::ImagePtr	imageptr = convertfile(image);
	return _repo.save(imageptr);
}

void    RepositoryI::remove(int id, const Ice::Current& /* current */) {
	_repo.remove(id);
}

} // namespace snowstar
