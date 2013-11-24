/*
 * GuiderFactory.cpp -- GuiderFactory implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroGuiding.h>
#include <AstroDevaccess.h>

using namespace astro::camera;
using namespace astro::module;
using namespace astro::device;

namespace astro {
namespace guiding {

//////////////////////////////////////////////////////////////////////
// GuiderDescriptor implementation
//////////////////////////////////////////////////////////////////////

/**
 * \brief Equality operator for GuiderDescriptor objects
 */
bool	GuiderDescriptor::operator==(const GuiderDescriptor& other) const {
	return (cameraname() == other.cameraname())
		&& (ccdid() == other.ccdid())
		&& (guiderportname() == other.guiderportname());
}

/**
 * \brief Comparison operator for GuiderDescriptor objects
 */
bool	GuiderDescriptor::operator<(const GuiderDescriptor& other) const {
	if (cameraname() < other.cameraname()) {
		return true;
	}
	if (cameraname() > other.cameraname()) {
		return false;
	}
	if (ccdid() < other.ccdid()) {
		return true;
	}
	if (ccdid() > other.ccdid()) {
		return false;
	}
	return guiderportname() < other.guiderportname();
}

std::string	GuiderDescriptor::toString() const {
	return stringprintf("%s|%d|%s", cameraname().c_str(), ccdid(),
		guiderportname().c_str());
}

//////////////////////////////////////////////////////////////////////
// GuiderFactory implementation
//////////////////////////////////////////////////////////////////////

/**
 * \brief Retrieve a list of known guiders
 */
std::vector<GuiderDescriptor>	GuiderFactory::list() const {
	std::vector<GuiderDescriptor>	result;
	guidermap_t::const_iterator	i;
	for (i = guiders.begin(); i != guiders.end(); i++) {
		result.push_back(i->first);
	}
	return result;
}

/**
 * \brief Get an existing guider or build a new one from the descriptor
 */
GuiderPtr	GuiderFactory::get(const GuiderDescriptor& guiderdescriptor) {
	guidermap_t::iterator	i = guiders.find(guiderdescriptor);
	if (i != guiders.end()) {
		return i->second;
	}

	// use the information in the descriptor to build a new guider
	CameraPtr	camera = cameraFromName(guiderdescriptor.cameraname());
	CcdPtr	ccd = camera->getCcd(guiderdescriptor.ccdid());
	GuiderPortPtr	guiderport;
	if (guiderdescriptor.guiderportname().size() == 0) {
		guiderport = camera->getGuiderPort();
	} else {
		guiderport = guiderportFromName(
			guiderdescriptor.guiderportname());
	}

	// with all these components we can now build a new guider
	GuiderPtr	guider(new Guider(camera, ccd, guiderport));
	return guider;
}

/**
 * \brief Get a camera from a repository based on the name
 */
CameraPtr	GuiderFactory::cameraFromName(const std::string& name) {
	Repository	repository;
	DeviceAccessor<CameraPtr>	da(repository);
	return da.get(DeviceName(name));
}

/**
 * \brief Get a guider port from a repository based on the name
 */
GuiderPortPtr	GuiderFactory::guiderportFromName(const std::string& name) {
	Repository	repository;
	DeviceAccessor<GuiderPortPtr>	da(repository);
	return da.get(DeviceName(name));
}

} // namespace guiding
} // namespace astro
