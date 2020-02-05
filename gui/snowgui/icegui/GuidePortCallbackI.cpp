/*
 * GuidePortCallbackI.cpp -- guideport callback implementation
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <guideportcontrollerwidget.h>
#include <IceConversions.h>

namespace snowgui {

GuidePortCallbackI::GuidePortCallbackI(guideportcontrollerwidget& g)
	: _guideportcontrollerwidget(g) {
	qRegisterMetaType<astro::camera::GuidePortActivation>(
		"astro::camera::GuidePortActivation");
}

void	GuidePortCallbackI::activate(const snowstar::GuidePortActivation& act,
		const Ice::Current& /* current */) {
	astro::camera::GuidePortActivation	a = convert(act);
	emit activation(a);
}

} // namespace snowgui



