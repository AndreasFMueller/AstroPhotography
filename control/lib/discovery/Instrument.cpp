/*
 * ServiceInstrument.cpp -- instrument implementation
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroDiscovery.h>
#include <AstroPersistence.h>
#include <AstroConfig.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <InstrumentComponentTable.h>
#include <InstrumentPropertyTable.h>

namespace astro {
namespace discover {

//////////////////////////////////////////////////////////////////////
// Instrument implementation
//////////////////////////////////////////////////////////////////////

/**
 * \brief Does the instrument have a component of this type
 */
bool	Instrument::has(InstrumentComponentKey::Type type) {
	return this->nComponentsOfType(type) > 0;
}

InstrumentComponent     Instrument::getAdaptiveOptics(int index) {
	return this->get(InstrumentComponentKey::AdaptiveOptics, index);
}

InstrumentComponent     Instrument::getCamera(int index) {
	return this->get(InstrumentComponentKey::Camera, index);
}

InstrumentComponent     Instrument::getCcd(int index) {
	return this->get(InstrumentComponentKey::CCD, index);
}

InstrumentComponent     Instrument::getCooler(int index) {
	return this->get(InstrumentComponentKey::Cooler, index);
}

InstrumentComponent     Instrument::getGuiderCcd(int index) {
	return this->get(InstrumentComponentKey::GuiderCCD, index);
}

InstrumentComponent     Instrument::getFinderCcd(int index) {
	return this->get(InstrumentComponentKey::FinderCCD, index);
}

InstrumentComponent     Instrument::getGuidePort(int index) {
	return this->get(InstrumentComponentKey::GuidePort, index);
}

InstrumentComponent     Instrument::getFilterWheel(int index) {
	return this->get(InstrumentComponentKey::FilterWheel, index);
}

InstrumentComponent     Instrument::getFocuser(int index) {
	return this->get(InstrumentComponentKey::Focuser, index);
}

InstrumentComponent     Instrument::getMount(int index) {
	return this->get(InstrumentComponentKey::Mount, index);
}

bool	Instrument::hasAdaptiveOptics() {
	return this->has(InstrumentComponentKey::AdaptiveOptics);
}

bool	Instrument::hasCamera() {
	return this->has(InstrumentComponentKey::Camera);
}

bool	Instrument::hasCcd() {
	return this->has(InstrumentComponentKey::CCD);
}

bool	Instrument::hasCooler() {
	return this->has(InstrumentComponentKey::Cooler);
}

bool	Instrument::hasGuiderCcd() {
	return this->has(InstrumentComponentKey::GuiderCCD);
}

bool	Instrument::hasFinderCcd() {
	return this->has(InstrumentComponentKey::FinderCCD);
}

bool	Instrument::hasGuidePort() {
	return this->has(InstrumentComponentKey::GuidePort);
}

bool	Instrument::hasFilterWheel() {
	return this->has(InstrumentComponentKey::FilterWheel);
}

bool	Instrument::hasFocuser() {
	return this->has(InstrumentComponentKey::Focuser);
}

bool	Instrument::hasMount() {
	return this->has(InstrumentComponentKey::Mount);
}

bool	Instrument::hasGuidername() {
	return hasGuiderCcd() && (hasGuidePort() || hasAdaptiveOptics());
}

astro::guiding::GuiderName	Instrument::guidername() {
	if (!hasGuidername()) {
		std::string	msg = stringprintf("instrument %s cannot guide",
			name().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	astro::guiding::GuiderName	guidername(name());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got GuiderName '%s'",
		guidername.instrument().c_str());
	return guidername;
}

astro::guiding::GuiderDescriptor	Instrument::guiderdescriptor() {
	if (!hasGuidername()) {
		std::string	msg = stringprintf("instrument %s cannot guide",
			name().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	astro::guiding::GuiderName	guidername = Instrument::guidername();
	std::string	ccd = getGuiderCcd().deviceurl();
	// check for guideport
	std::string	guideport;
	if (guidername.hasGuidePort()) {
		guideport = getGuidePort().deviceurl();
	}
	std::string	adaptiveoptics;
	if (guidername.hasAdaptiveOptics()) {
		adaptiveoptics = getAdaptiveOptics().deviceurl();
	}
	astro::guiding::GuiderDescriptor	guiderdescriptor(
		guidername.instrument(), ccd, guideport, adaptiveoptics);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got guiderdescriptor '%s'",
		guiderdescriptor.toString().c_str());
	return guiderdescriptor;
}

/**
 * \brief Auxiliary function to add components to a list
 */
void	Instrument::add(std::list<InstrumentComponent>& l,
		InstrumentComponent::Type type) {
	int	n = nComponentsOfType(type);
	for (int i = 0; i < n; i++) {
		l.push_back(get(type, i));
	}
}

/**
 * \brief Build a list of all components of an instrument
 */
std::list<InstrumentComponent>	Instrument::list() {
	std::list<InstrumentComponent>	result;
	add(result, InstrumentComponent::AdaptiveOptics);
	add(result, InstrumentComponent::Camera);
	add(result, InstrumentComponent::CCD);
	add(result, InstrumentComponent::Cooler);
	add(result, InstrumentComponent::GuiderCCD);
	add(result, InstrumentComponent::FinderCCD);
	add(result, InstrumentComponent::GuidePort);
	add(result, InstrumentComponent::FilterWheel);
	add(result, InstrumentComponent::Focuser);
	add(result, InstrumentComponent::Mount);
	return result;
}

int	Instrument::getInt(const std::string& name) {
	return std::stoi(getProperty(name).value());
}

double	Instrument::getDouble(const std::string& name) {
	return std::stod(getProperty(name).value());
}

std::string	Instrument::getString(const std::string& name) {
	return getProperty(name).value();
}

/**
 * \brief Build a list of all components of a given type
 */
std::list<InstrumentComponent>	Instrument::list(InstrumentComponent::Type type) {
	std::list<InstrumentComponent>	result;
	add(result, type);
	return result;
}

} // namespace discover
} // namespace astro
