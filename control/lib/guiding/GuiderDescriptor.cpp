/*
 * GuiderDescriptor.cpp -- GuiderDescriptor implementation
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
	if (name() < other.name()) {
		return true;
	}
	if (name() > other.name()) {
		return false;
	}
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
	if (guiderport() < other.guiderport()) {
		return true;
	}
	if (guiderport() > other.guiderport()) {
		return false;
	}
	return adaptiveoptics() < other.adaptiveoptics();
}

std::string	GuiderDescriptor::toString() const {
	return stringprintf("%s:%s|%s|%s", name().c_str(),
		instrument().c_str(), ccd().c_str(),
		guiderport().c_str(), adaptiveoptics().c_str());
}

} // namespace guiding
} // namespace astro
