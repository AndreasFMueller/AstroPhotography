/*
 * guidercontrollerwidget.cpp -- implementation of the guider controller
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "guidercontrollerwidget.h"
#include "ui_guidercontrollerwidget.h"
#include <QTimer>
#include <CommunicatorSingleton.h>
#include <IceConversions.h>

namespace snowgui {

guidercontrollerwidget::guidercontrollerwidget(QWidget *parent)
	: InstrumentWidget(parent), ui(new Ui::guidercontrollerwidget) {
	ui->setupUi(this);

	// adding items to the tracking metho
	ui->methodBox->addItem(QString("Star"));
	ui->methodBox->addItem(QString("Phase"));
	ui->methodBox->addItem(QString("Gradient"));
	ui->methodBox->addItem(QString("Laplace"));
	ui->methodBox->addItem(QString("Large"));

	// some other fields
	statusTimer = new QTimer;
	statusTimer->setInterval(100);
	_guiderportinterval = 10;
	_adaptiveopticsinterval = 1;
	_stepping = false;
}

/**
 * \brief Instrument setup for the guidercontrollerwidget
 *
 * This method also creates the guider factory and the guidre descriptor.
 * The guider itself is set up in setGuider() method
 */
void	guidercontrollerwidget::instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument) {
	InstrumentWidget::instrumentSetup(serviceobject, instrument);

	// create the guiderfactory proxy
	Ice::CommunicatorPtr    ic = snowstar::CommunicatorSingleton::get();
	astro::ServerName	servername(serviceobject.name());
	Ice::ObjectPrx  gbase
		= ic->stringToProxy(servername.connect("Guiders"));
	_guiderfactory = snowstar::GuiderFactoryPrx::checkedCast(gbase);

	// now build a GuiderDescriptor for the guider
	_guiderdescriptor.instrumentname = _instrument.name();
	_guiderdescriptor.ccdIndex = 0;
	_guiderdescriptor.guiderportIndex = 0;
	_guiderdescriptor.adaptiveopticsIndex = 0;

	// set up the guider
	setupGuider();
}

/**
 * \brief Setup a guider
 */
void	guidercontrollerwidget::setupGuider() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setting up the guider %s|%d|%d|%d",
		_guiderdescriptor.instrumentname.c_str(),
		_guiderdescriptor.ccdIndex, _guiderdescriptor.guiderportIndex,
		_guiderdescriptor.adaptiveopticsIndex);
	statusTimer->stop();

	// get the guider based on the descriptor
	_guider = _guiderfactory->get(_guiderdescriptor);

	// also propagate the information to the calibration widgets
	ui->gpcalibrationWidget->setGuider(snowstar::ControlGuiderPort,
		_guiderdescriptor, _guider, _guiderfactory);
	ui->aocalibrationWidget->setGuider(snowstar::ControlAdaptiveOptics,
		_guiderdescriptor, _guider, _guiderfactory);

	// start the timer
	statusTimer->start();
}

/**
 * \brief This 
 */
guidercontrollerwidget::~guidercontrollerwidget() {
	delete statusTimer;
	delete ui;
}

/**
 * \brief Set the exposure to use for the guider
 */
void	guidercontrollerwidget::setExposure(astro::camera::Exposure exposure) {
	_exposure = exposure;
	try {
		_guider->setExposure(snowstar::convert(_exposure));
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "could not set exposure");
	}
}

/**
 * \brief Slot to change the star
 */
void	guidercontrollerwidget::setStar(ImagePoint star) {
	_star = star;
	try {
		snowstar::Point	p;
		p.x = star.x();
		p.y = star.y();
		_guider->setStar(p);
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot set star");
	}
}

void	guidercontrollerwidget::setCcd(int index) {
	_guiderdescriptor.ccdIndex = index;
	setupGuider();
}

void	guidercontrollerwidget::setGuiderport(int index) {
	_guiderdescriptor.guiderportIndex = index;
	setupGuider();
}

void	guidercontrollerwidget::setAdaptiveoptics(int index) {
	_guiderdescriptor.adaptiveopticsIndex = index;
	setupGuider();
}

void	guidercontrollerwidget::startCalibration() {
	if (!_guider) {
		return;
	}
	try {
		// XXX we need to know which device to calibrate
		_guider->startCalibration(snowstar::ControlGuiderPort);
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot start the calibration");
	}
}

void	guidercontrollerwidget::cancelCalibration() {
	if (!_guider) {
		return;
	}
	try {
		_guider->cancelCalibration();
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot cancel the calibration");
	}
}

void	guidercontrollerwidget::startGuiding() {
	if (!_guider) {
		return;
	}
	try {
		_guider->startGuiding(_guiderportinterval,
			_adaptiveopticsinterval, _stepping);
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot start guiding");
	}
}

void	guidercontrollerwidget::stopGuiding() {
	if (!_guider) {
		return;
	}
	try {
		_guider->stopGuiding();
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot stop guiding");
	}
}

/**
 * \brief Slot to update the status display
 */
void	guidercontrollerwidget::statusUpdate() {
	if (!_guider) {
		return;
	}
	snowstar::GuiderState	state = _guider->getState();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "state: %d", (int)state);
}

} // namespade snowgui
