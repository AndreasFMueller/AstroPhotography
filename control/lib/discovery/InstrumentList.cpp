/*
 * InstrumentList.cpp -- instrument implementation
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroDiscovery.h>

namespace astro {
namespace discover {

InstrumentList::InstrumentList(const std::list<std::string>& list) {
	std::list<std::string>::const_iterator	i;
	for (i = list.begin(); i != list.end(); i++) {
		push_back(*i);
	}
}

} // namespace discover
} // namespace astro
