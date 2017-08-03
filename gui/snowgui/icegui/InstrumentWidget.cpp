/*
 * InstrumentWidget.cpp -- implementation of the instrument widget
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <InstrumentWidget.h>
#include <ImageForwarder.h>

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

	// connect this object to the image forwarder
	connect(this,
		SIGNAL(offerImage(astro::image::ImagePtr, std::string)),
		ImageForwarder::get(),
		SLOT(sendImage(astro::image::ImagePtr, std::string)));

	// get the instrument name into the title
	std::string     title
		= astro::stringprintf("Instrument %s @ %s",
		_instrument.name().c_str(), serviceobject.toString().c_str());
	setWindowTitle(QString(title.c_str()));

	debug(LOG_DEBUG, DEBUG_LOG, 0, "window starting on instrument %s",
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

void	InstrumentWidget::sendImage(astro::image::ImagePtr image,
		std::string title) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "sendImage, title = '%s'",
		title.c_str());
	_image = image;
	_title = title;
	emit offerImage(_image, _title);
}

void	InstrumentWidget::changeEvent(QEvent *event) {
	if (this->window()->isActiveWindow()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "offering '%s'", _title.c_str());
		emit offerImage(_image, _title);
	}
	QWidget::changeEvent(event);
}

} // namespace snowgui
