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
	if (guiderdescriptor.ccd().size()) {
		DeviceAccessor<CcdPtr>	da(repository);
		ccd = da.get(DeviceName(guiderdescriptor.ccd()));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "ccd constructed");
	}
	GuiderPortPtr	guiderport;
	if (guiderdescriptor.guiderport().size()) {
		DeviceAccessor<GuiderPortPtr>	da(repository);
		guiderport = da.get(DeviceName(guiderdescriptor.guiderport()));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "guiderport constructed");
	}
	AdaptiveOpticsPtr	adaptiveoptics;
	if (guiderdescriptor.adaptiveoptics().size()) {
		DeviceAccessor<AdaptiveOpticsPtr>	da(repository);
		adaptiveoptics = da.get(DeviceName(guiderdescriptor.ccd()));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "adaptiveoptics constructed");
	}

	// with all these components we can now build a new guider
	GuiderPtr	guider(new Guider(guiderdescriptor.instrument(),
				ccd, guiderport, database));
	guiders.insert(std::make_pair(guiderdescriptor, guider));
	return guider;
}

} // namespace guiding
} // namespace astro
