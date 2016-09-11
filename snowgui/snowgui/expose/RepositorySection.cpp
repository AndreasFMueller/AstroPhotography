/*
 * RepositorySection.cpp
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <RepositorySection.h>
#include <IceConversions.h>
#include <AstroCamera.h>

namespace snowgui {

static std::string	purpose2string(snowstar::ExposurePurpose purpose) {
	return astro::camera::Exposure::purpose2string(convert(purpose));
}

static snowstar::ExposurePurpose string2purpose(const std::string& purpose) {
	return snowstar::convert(
		astro::camera::Exposure::string2purpose(purpose));
}

//////////////////////////////////////////////////////////////////////
// RepositoryKey implementation
//////////////////////////////////////////////////////////////////////
RepositoryKey::RepositoryKey(snowstar::ExposurePurpose purpose,
		const std::string& filtername)
	: _purpose(purpose), _filtername(filtername) {
}

RepositoryKey::RepositoryKey(snowstar::ExposurePurpose purpose)
	: _purpose(purpose) {
}

RepositoryKey::RepositoryKey(const std::string& purpose)
	: _purpose(string2purpose(purpose)) {
}

RepositoryKey::RepositoryKey(const std::string& purpose,
	const std::string& filtername)
	: _purpose(string2purpose(purpose)), _filtername(filtername) {
}


RepositoryKey::RepositoryKey(const RepositoryKey& key) 
	: _purpose(key._purpose), _filtername(key._filtername) {
}

std::string	RepositoryKey::purposeString() const {
	return purpose2string(_purpose);
}

std::string	RepositoryKey::toString() const {
	return astro::stringprintf("%s|%s", purposeString().c_str(),
		_filtername.c_str());
}

bool	RepositoryKey::operator<(const RepositoryKey& key) const {
	if (_purpose < key._purpose) {
		return true;
	}
	if (_purpose > key._purpose) {
		return false;
	}
	if (_filtername < key._filtername) {
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////
// RepositorySection implementation
//////////////////////////////////////////////////////////////////////
RepositorySection::RepositorySection(snowstar::ExposurePurpose purpose,
	const std::string& filtername, int filterindex)
	: RepositoryKey(purpose, filtername), _filterindex(filterindex) {
	_index = -1;
}

RepositorySection::RepositorySection(snowstar::ExposurePurpose purpose)
	: RepositoryKey(purpose) {
	_filterindex = -1;
	_index = -1;
}

RepositorySection::RepositorySection(const RepositoryKey& key, int filterindex,
	int index)
	: RepositoryKey(key), _filterindex(filterindex), _index(index) {
}

RepositorySection::RepositorySection(const RepositoryKey& key, int index)
	: RepositoryKey(key), _filterindex(-1), _index(index) {
}

} // namespace snowgui
