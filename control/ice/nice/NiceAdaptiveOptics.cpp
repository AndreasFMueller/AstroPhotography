/*
 * NiceAdaptiveOptics.cpp
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <NiceAdaptiveOptics.h>
#include <CommunicatorSingleton.h>
#include <IceConversions.h>

namespace astro {
namespace camera {
namespace nice {

void	NiceAdaptiveOpticsCallbackI::point(const snowstar::Point& p,
			const Ice::Current& /* current */) {
	_adaptiveoptics.callback(convert(p));
}

void	NiceAdaptiveOpticsCallbackI::stop(const Ice::Current& /* current */) {
}

NiceAdaptiveOptics::NiceAdaptiveOptics(
	snowstar::AdaptiveOpticsPrx adaptiveoptics,
	const DeviceName& devicename)
	: AdaptiveOptics(devicename), NiceDevice(devicename),
	  _adaptiveoptics(adaptiveoptics) {
	_hasguideport = adaptiveoptics->hasGuidePort();
	_adaptiveoptics_callback = new NiceAdaptiveOpticsCallbackI(*this);
	_adaptiveoptics_identity = snowstar::CommunicatorSingleton::add(
		_adaptiveoptics_callback);
	_adaptiveoptics->registerCallback(_adaptiveoptics_identity);
}

NiceAdaptiveOptics::~NiceAdaptiveOptics() {
	_adaptiveoptics->unregisterCallback(_adaptiveoptics_identity);
	 snowstar::CommunicatorSingleton::remove(_adaptiveoptics_identity);
}

void	NiceAdaptiveOptics::set0(const Point& position) {
	_adaptiveoptics->set(snowstar::convert(position));
}

GuidePortPtr	NiceAdaptiveOptics::getGuidePort0() {
	snowstar::GuidePortPrx	guideport = _adaptiveoptics->getGuidePort();
	return GuidePortPtr(new NiceGuidePort(guideport,
		nice(guideport->getName())));
}

} // namespace nice
} // namespace camera
} // namespace astro
