/*
 * NiceAdaptiveOptics.cpp
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <NiceAdaptiveOptics.h>
#include <IceConversions.h>

namespace astro {
namespace camera {
namespace nice {

NiceAdaptiveOptics::NiceAdaptiveOptics(
	snowstar::AdaptiveOpticsPrx adaptiveoptics,
	const DeviceName& devicename)
	: AdaptiveOptics(devicename), NiceDevice(devicename),
	  _adaptiveoptics(adaptiveoptics) {
	_hasguideport = adaptiveoptics->hasGuidePort();
}

NiceAdaptiveOptics::~NiceAdaptiveOptics() {
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
