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
	_hasguiderport = adaptiveoptics->hasGuiderPort();
}

NiceAdaptiveOptics::~NiceAdaptiveOptics() {
}

void	NiceAdaptiveOptics::set0(const Point& position) {
	_adaptiveoptics->set(snowstar::convert(position));
}

GuiderPortPtr	NiceAdaptiveOptics::getGuiderPort0() {
	snowstar::GuiderPortPrx	guiderport = _adaptiveoptics->getGuiderPort();
	return GuiderPortPtr(new NiceGuiderPort(guiderport,
		nice(guiderport->getName())));
}

} // namespace nice
} // namespace camera
} // namespace astro
