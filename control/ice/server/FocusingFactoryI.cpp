/*
 * FocusingFactoryI.cpp -- implementation of the focusing factory
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <FocusingFactoryI.h>
#include <AstroFormat.h>
#include <ProxyCreator.h>
#include <IceConversions.h>
#include <AstroUtils.h>

namespace snowstar {

std::mutex	factory_mutex;

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

	// now use the data to create a new focusing context
	FocusingContext	context;
	context.id = nextid;

	// insert the context in the map
	focusings.insert(std::make_pair(key, context));

	// return the context created
	return context;
}

/**
 * \brief factory method to retrieve focusing context identified by id
 */
FocusingContext	FocusingSingleton::get(int id) {
	astro::MutexLocker<std::mutex>	lock(factory_mutex);
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
	return createProxy<FocusingPrx>(focusingname, current);
}

} // namespace snowstar
