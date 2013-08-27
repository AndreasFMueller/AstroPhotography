/*
 * Focuser.cpp -- Focuser base class
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCamera.h>
#include <AstroExceptions.h>

using namespace astro::device;

namespace astro {
namespace camera {

Focuser::Focuser(const std::string name) : Device(name) {
}

Focuser::~Focuser() {
}

unsigned short	Focuser::min() {
	return 0;
}

unsigned short	Focuser::max() {
	return std::numeric_limits<unsigned short>::max();
}

unsigned short	Focuser::current() {
	throw NotImplemented("base Focuser does not implement current method");
}

void	Focuser::set(unsigned short value) {
	throw NotImplemented("base Focuser does not implement set method");
}

} // namespace camera
} // namespace astro
