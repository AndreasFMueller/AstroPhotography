/*
 * GuidePortI.cpp --
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <GuidePortI.h>
#include <NameConverter.h>
#include <ProxyCreator.h>

namespace snowstar {

GuidePortI::GuidePortI(astro::camera::GuidePortPtr guideport)
		: DeviceI(*guideport), _guideport(guideport) {
}

GuidePortI::~GuidePortI() {
}

Ice::Byte	GuidePortI::active(const Ice::Current& current) {
	CallStatistics::count(current);
	return _guideport->active();
}

void	GuidePortI::activate(float ra, float dec, const Ice::Current& current) {
	CallStatistics::count(current);
	double  raplus = 0, raminus = 0, decplus = 0, decminus = 0;
	if (ra > 0) {
		raplus = ra;
	} else {
		raminus = -ra;
	}
	if (dec > 0) {
		decplus = dec;
	} else {
		decminus = -dec;
	}
	_guideport->activate(raplus, raminus, decplus, decminus);
}

GuidePortPrx	GuidePortI::createProxy(const std::string& guideportname,
			const Ice::Current& current) {
	return snowstar::createProxy<GuidePortPrx>(guideportname, current);
}

} // namespace snowstar
