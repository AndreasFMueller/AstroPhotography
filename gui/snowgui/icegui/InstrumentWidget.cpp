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
	qRegisterMetaType<std::string>("std::string");
}

InstrumentWidget::~InstrumentWidget() {
}

/**
 * \brief Common stuff for instrument setup
 */
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
	std::string     title = astro::stringprintf("Instrument %s @ %s",
		_instrument.name().c_str(), serviceobject.toString().c_str());
	setWindowTitle(QString(title.c_str()));

	std::string     t = astro::demangle(typeid(*this).name());

	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s starting on instrument %s",
		t.c_str(), instrumentname().c_str());
}

/**
 * \brief common instrument setup completion 
 */
void	InstrumentWidget::setupComplete() {
	std::string     t = astro::demangle(typeid(*this).name());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s setup complete", t.c_str());
}

/**
 * \brief Slot to handle the completion signal from the setup thread
 */
void	InstrumentWidget::setupCompletion() {
	std::string     t = astro::demangle(typeid(*this).name());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s setupCompletion()", t.c_str());
	this->setupComplete();
}

/**
 * \brief Start the instrument setup thread
 */
void	InstrumentWidget::launchInstrumentSetup(
				astro::discover::ServiceObject serviceobject,
                                snowstar::RemoteInstrument instrument) {
	// start a thread for the setup
	// start the instrument setup thread
        InstrumentSetupThread   *setupthread
                = new InstrumentSetupThread(this, instrument, serviceobject);
        setupthread->start();
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
