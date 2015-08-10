/*
 * GuiderPortI.cpp --
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <GuiderPortI.h>
#include <NameConverter.h>
#include <ProxyCreator.h>

namespace snowstar {

GuiderPortI::GuiderPortI(astro::camera::GuiderPortPtr guiderport)
		: DeviceI(*guiderport), _guiderport(guiderport) {
}

GuiderPortI::~GuiderPortI() {
}

Ice::Byte	GuiderPortI::active(const Ice::Current& /* current */) {
	return _guiderport->active();
}

void	GuiderPortI::activate(float ra, float dec, const Ice::Current& /* current */) {
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
	_guiderport->activate(raplus, raminus, decplus, decminus);
}

GuiderPortPrx	GuiderPortI::createProxy(const std::string& guiderportname,
			const Ice::Current& current) {
	return snowstar::createProxy<GuiderPortPrx>(guiderportname, current);
}

} // namespace snowstar
