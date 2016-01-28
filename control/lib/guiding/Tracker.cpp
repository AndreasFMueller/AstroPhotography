/*
 * Tracker.cpp -- tracker base class methods implementation
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroGuiding.h>
#include <AstroUtils.h>

namespace astro {
namespace guiding {

std::string	Tracker::toString() const {
	return demangle(typeid(this).name());
}

} // namespace guiding
} // namespace astro
