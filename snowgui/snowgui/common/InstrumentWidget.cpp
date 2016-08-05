/*
 * InstrumentWidget.cpp -- implementation of the instrument widget
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <InstrumentWidget.h>

using namespace astro::discover;

namespace snowgui {

InstrumentWidget::InstrumentWidget(QWidget *parent) : QWidget(parent) {
}

InstrumentWidget::~InstrumentWidget() {
}

void	InstrumentWidget::instrumentSetup(ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument) {
	// remember service object and instrument
	_servicekey = serviceobject;
	_instrument = instrument;

	// get the instrument name into the title
	std::string     title
		= astro::stringprintf("Instrument %s @ %s",
		_instrument.name().c_str(), serviceobject.toString().c_str());
	setWindowTitle(QString(title.c_str()));

	debug(LOG_DEBUG, DEBUG_LOG, 0, "preview starting on instrument %s",
		instrumentname().c_str());
}

std::string	InstrumentWidget::instrumentname() {
	return _instrument.name();
}

void	InstrumentWidget::setAppname(const std::string& a) {
	_appname = a;
	std::string     title
		= astro::stringprintf("%s using instrument %s @ %s",
			_appname.c_str(), _instrument.name().c_str(),
			_servicekey.toString().c_str());
	setWindowTitle(QString(title.c_str()));
}

} // namespace snowgui
