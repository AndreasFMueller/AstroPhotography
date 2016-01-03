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
	return (instrument() == other.instrument())
		&& (ccd() == other.ccd())
		&& (guiderport() == other.guiderport());
}

/**
 * \brief Comparison operator for GuiderDescriptor objects
 */
bool	GuiderDescriptor::operator<(const GuiderDescriptor& other) const {
	if (instrument() < other.instrument()) {
		return true;
	}
	if (instrument() > other.instrument()) {
		return false;
	}
	if (ccd() < other.ccd()) {
		return true;
	}
	if (ccd() > other.ccd()) {
		return false;
	}
	return guiderport() < other.guiderport();
}

std::string	GuiderDescriptor::toString() const {
	return stringprintf("%s|%s|%s", instrument().c_str(), ccd().c_str(),
		guiderport().c_str());
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "check whether guider is in cache");
	guidermap_t::iterator	i = guiders.find(guiderdescriptor);
	if (i != guiders.end()) {
		return i->second;
	}

	// use the information in the descriptor to build a new guider
	Repository	repository;
	CcdPtr	ccd;
	{
		DeviceAccessor<CcdPtr>	da(repository);
		ccd = da.get(DeviceName(guiderdescriptor.ccd()));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "ccd constructed");
	}
	GuiderPortPtr	guiderport;
	{
		DeviceAccessor<GuiderPortPtr>	da(repository);
		guiderport = da.get(DeviceName(guiderdescriptor.guiderport()));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "components constructed");
	}

	// with all these components we can now build a new guider
	GuiderPtr	guider(new Guider(guiderdescriptor.instrument(),
				ccd, guiderport, database));
	guiders.insert(std::make_pair(guiderdescriptor, guider));
	return guider;
}

#if 0
/**
 * \brief Get a camera from a repository based on the name
 */
CcdPtr	GuiderFactory::ccdFromName(const std::string& name) {
	Repository	repository;
	DeviceAccessor<CcdPtr>	da(repository);
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
#endif

} // namespace guiding
} // namespace astro
