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
		GuiderPtr	guider = i->second;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found guider '%s' in cache",
			guider->name().c_str());
		// XXX we should update the parameters, because they
		// XXX  might have changed
		return guider;
	}

	// construct the name of the guider
	GuiderName	guidername(guiderdescriptor.name());

	// first get a module repository
	ModuleRepositoryPtr	repository = getModuleRepository();
	if (!repository) {
		debug(LOG_ERR, DEBUG_LOG, 0, "no module repository");
		throw std::logic_error("no module repository");
	}

	// use the information in the descriptor to build a new guider
	CcdPtr	ccd;
	if (guiderdescriptor.ccd().size()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "get CCD %s for guider %s",
			guiderdescriptor.ccd().c_str(),
			guidername.name().c_str());
		DeviceAccessor<CcdPtr>	da(repository);
		ccd = da.get(DeviceName(guiderdescriptor.ccd()));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "ccd constructed");
	} else {
		std::string	msg = stringprintf("Guider %s has no CCD",
			guiderdescriptor.toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	GuidePortPtr	guideport;
	if (guiderdescriptor.guideport().size()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "get GuidePort %s for guider %s",
			guiderdescriptor.guideport().c_str(),
			guidername.name().c_str());
		DeviceAccessor<GuidePortPtr>	da(repository);
		guideport = da.get(DeviceName(guiderdescriptor.guideport()));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "guideport constructed");
	} else {
		std::string	msg = stringprintf("Guider %s has no Port",
			guiderdescriptor.toString().c_str());
		debug(LOG_WARNING, DEBUG_LOG, 0, "%s", msg.c_str());
	}
	AdaptiveOpticsPtr	adaptiveoptics;
	if (guiderdescriptor.adaptiveoptics().size()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "get AO %s for guider %s",
			guiderdescriptor.adaptiveoptics().c_str(),
			guidername.name().c_str());
		DeviceAccessor<AdaptiveOpticsPtr>	da(repository);
		adaptiveoptics = da.get(DeviceName(guiderdescriptor.adaptiveoptics()));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "adaptiveoptics constructed");
	} else {
		std::string	msg = stringprintf("Guider %s has no AO",
			guiderdescriptor.toString().c_str());
		debug(LOG_INFO, DEBUG_LOG, 0, "%s", msg.c_str());
	}

	// with all these components we can now build a new guider
	GuiderPtr	guider(new Guider(guidername,
				ccd, guideport, adaptiveoptics, database));
	guiders.insert(std::make_pair(guiderdescriptor, guider));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "return guider '%s'",
		guider->name().c_str());
	return guider;
}

} // namespace guiding
} // namespace astro
