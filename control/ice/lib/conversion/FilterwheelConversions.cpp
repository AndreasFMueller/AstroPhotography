/*
 * FilterwheelConversions.cpp -- conversions between ice and astro
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <IceConversions.h>
#include <includes.h>
#include <AstroIO.h>
#include <AstroUtils.h>

namespace snowstar {

// Filterwheel
FilterwheelState	convert(const astro::camera::FilterWheel::State& s) {
	switch (s) {
	case astro::camera::FilterWheel::idle:
		return snowstar::FwIDLE;
	case astro::camera::FilterWheel::moving:
		return snowstar::FwMOVING;
	case astro::camera::FilterWheel::unknown:
		return snowstar::FwUNKNOWN;
	}
	throw std::runtime_error("unknown filterwheel state");
}

astro::camera::FilterWheel::State convert(const FilterwheelState& s) {
	switch (s) {
	case snowstar::FwIDLE:
		return astro::camera::FilterWheel::idle;
	case snowstar::FwMOVING:
		return astro::camera::FilterWheel::moving;
	case snowstar::FwUNKNOWN:
		return astro::camera::FilterWheel::unknown;
	}
	throw std::runtime_error("unknown filterwheel state");
}

} // namespace snowstar
