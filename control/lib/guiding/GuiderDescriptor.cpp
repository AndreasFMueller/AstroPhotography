/*
 * GuiderDescriptor.cpp -- GuiderDescriptor implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroGuiding.h>
#include <AstroDevaccess.h>
#include <AstroDiscovery.h>

using namespace astro::camera;
using namespace astro::module;
using namespace astro::device;

namespace astro {
namespace guiding {

/**
 * \brief Read compnents from the database
 *
 * \param instrumentname	name of the instrument
 */
void	GuiderDescriptor::setup(const std::string& instrumentname) {
	_instrument = instrumentname;
	// get the Instrument
	discover::InstrumentPtr	instrument
		= discover::InstrumentBackend::get(instrumentname);
	if (!instrument) {
		std::string	msg = stringprintf("no instrument '%s'",
			instrumentname.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// if the instrument has no guiderccd, we cannot build a guider
	if (!instrument->hasGuiderCcd()) {
		std::string	msg = stringprintf("no guiderccd in '%s'",
			instrumentname.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	_ccd = instrument->getGuiderCcd(0).deviceurl();
	if (instrument->hasGuidePort())  {
		_guideport = instrument->getGuidePort(0).deviceurl();
	}
	if (instrument->hasAdaptiveOptics()) {
		_adaptiveoptics = instrument->getAdaptiveOptics(0).deviceurl();
	}
}

/**
 * \brief Refresh the information in the descriptor by rereading the database
 */
void	GuiderDescriptor::refresh() {
	setup(_instrument);
}

/**
 * \brief Construct a guider descriptor using instrument data
 * 
 * \param instrumentname	name of the instrument	
 */
GuiderDescriptor::GuiderDescriptor(const std::string& instrumentname) {
	setup(instrumentname);
}

/**
 * \brief Equality operator for GuiderDescriptor objects
 */
bool	GuiderDescriptor::operator==(const GuiderDescriptor& other) const {
	return (instrument() == other.instrument())
		&& (ccd() == other.ccd())
		&& (guideport() == other.guideport());
}

/**
 * \brief Comparison operator for GuiderDescriptor objects
 */
bool	GuiderDescriptor::operator<(const GuiderDescriptor& other) const {
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
	if (guideport() < other.guideport()) {
		return true;
	}
	if (guideport() > other.guideport()) {
		return false;
	}
	return adaptiveoptics() < other.adaptiveoptics();
}

/**
 * \brief Convert a GuiderDescriptor to a string
 */
std::string	GuiderDescriptor::toString() const {
	return stringprintf("%s:%s|%s|%s",
		instrument().c_str(), ccd().c_str(),
		guideport().c_str(), adaptiveoptics().c_str());
}

} // namespace guiding
} // namespace astro
