/*
 * FocusingFactoryI.cpp -- implementation of the focusing factory
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <FocusingFactoryI.h>
#include <AstroFormat.h>
#include <ProxyCreator.h>
#include <IceConversions.h>
#include <AstroDevaccess.h>
#include <AstroUtils.h>
#include <FocusingI.h>

namespace snowstar {

std::string	FocusingKey::toString() const {
	return ccd() + " " + focuser();
}

std::mutex	factory_mutex;

FocusingSingleton::FocusingMap	FocusingSingleton::focusings;

/**
 * \brief factory method for focusing contexts
 *
 * This method gets a focusing context from the map if it already exists,
 * or creates one if there is none.
 */
FocusingContext	FocusingSingleton::get(const std::string& ccd,
	const std::string& focuser) {
	FocusingKey	key(ccd, focuser);

	astro::MutexLocker<std::mutex>	lock(factory_mutex);

	// find the focusing context
	auto ptr = focusings.find(key);
	if (ptr != focusings.end()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found existing %s",
			key.toString().c_str());
		return ptr->second;
	}

	// apparently there is no such context yet, so we must create a new
	// one. First we need to find the largest of the id values, because
	// the new context gets a larger id
	int	nextid = 0;
	if (focusings.size() > 0) {
		nextid = std::max_element(focusings.begin(), focusings.end(),
			[](const FocusingMap::value_type& a,
				const FocusingMap::value_type& b) {
				return a.second.id < b.second.id;
			}
		)->second.id;
	}

	// we need pointers for deives
	astro::module::ModuleRepositoryPtr	repository = astro::module::getModuleRepository();
	astro::module::Devices	devices(repository);

	astro::DeviceName	ccdname(ccd);
	astro::camera::CcdPtr	ccdptr
		= devices.getCcd(ccdname);

	astro::DeviceName	focusername(focuser);
	astro::camera::FocuserPtr	focuserptr
		= devices.getFocuser(focusername);

	// now use the data to create a new focusing context
	FocusingContext	context;
	context.id = nextid;
	context.focusing = astro::focusing::FocusingPtr(
		new astro::focusing::Focusing(ccdptr, focuserptr));
	context.focusingptr = new FocusingI(context.focusing);

	// insert the context in the map
	focusings.insert(std::make_pair(key, context));

	// return the context created
	return context;
}

/**
 * \brief factory method to retrieve focusing context identified by id
 */
FocusingContext	FocusingSingleton::get(int id) {
	// ensure exclusive access to the focusing map
	astro::MutexLocker<std::mutex>	lock(factory_mutex);

	// search the map for an entry with a given id
	auto ptr = std::find_if(focusings.begin(), focusings.end(),
		[id](const FocusingMap::value_type& focusing) {
			return (focusing.second.id == id);
		}
	);
	if (ptr == focusings.end()) {
		throw std::runtime_error("focusing not found");
	}
	return ptr->second;
}

/**
 * \brief Build a focusing proxy
 */
FocusingPrx	FocusingFactoryI::get(const std::string& ccd,
		const std::string& focuser, const Ice::Current& current) {
	FocusingContext	ctx = FocusingSingleton::get(ccd, focuser);
	std::string	focusingname = astro::stringprintf("focusing/%d",
		ctx.id);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "created proxy: %s",
		focusingname.c_str());
	return createProxy<FocusingPrx>(focusingname, current, false);
}

FocusingFactoryI::FocusingFactoryI() {
}

FocusingFactoryI::~FocusingFactoryI() {
}

} // namespace snowstar
