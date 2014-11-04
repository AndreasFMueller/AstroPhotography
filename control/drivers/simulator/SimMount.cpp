/*
 * SimMount.cpp -- simulated mount implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SimMount.h>

using namespace astro::device;

namespace astro {
namespace camera {
namespace simulator {

SimMount::SimMount(SimLocator& locator) 
	 : Mount(DeviceName("mount:simulator/mount")), _locator(locator) {
}

Mount::state_type	SimMount::state() {
	return Mount::IDLE;
}

RaDec	SimMount::getRaDec() {
	throw std::runtime_error("XXX cannot get RaDec");
}

AzmAlt	SimMount::getAzmAlt() {
	throw std::runtime_error("XXX cannot get AzmAlt");
}

void	SimMount::Goto(const RaDec& /* radec */) {
	throw std::runtime_error("XXX cannot goto RaDec");
}

void	SimMount::Goto(const AzmAlt& /* azmalt */) {
	throw std::runtime_error("XXX cannot goto AzmAlt");
}

void	SimMount::cancel() {
	// XXX implementation missing
}

} // namespace simulator
} // namespace camera
} // namespace astro
