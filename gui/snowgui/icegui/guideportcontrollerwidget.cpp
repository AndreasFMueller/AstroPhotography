/*
 * guideportcontrollerwidget.cpp 
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rappeswil
 */
#include "guideportcontrollerwidget.h"
#include "ui_guideportcontrollerwidget.h"
#include <camera.h>

namespace snowgui {

guideportcontrollerwidget::guideportcontrollerwidget(QWidget *parent)
	: InstrumentWidget(parent),
	  ui(new Ui::guideportcontrollerwidget) {
	ui->setupUi(this);
	ui->guideWidget->setEnabled(false);
	ui->activationWidget->setEnabled(false);

	// guiderate
	_guiderate = 0.5;

	// connect signals
	connect(ui->guideportSelectionBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(guideportChanged(int)));

	connect(ui->guiderButton, SIGNAL(westClicked()),
		this, SLOT(activateRAplus()));
	connect(ui->guiderButton, SIGNAL(eastClicked()),
		this, SLOT(activateRAminus()));
	connect(ui->guiderButton, SIGNAL(northClicked()),
		this, SLOT(activateDECplus()));
	connect(ui->guiderButton, SIGNAL(southClicked()),
		this, SLOT(activateDECminus()));

	connect(ui->activationtimeSpinBox, SIGNAL(valueChanged(double)),
		this, SLOT(changeActivationTime(double)));

	connect(ui->activateButton, SIGNAL(clicked()),
		this, SLOT(activateClicked()));

	// start the timer
	_active = 0;
	connect(&_statusTimer, SIGNAL(timeout()), this, SLOT(statusUpdate()));
	_statusTimer.setInterval(100);

	// set default activation time
	_activationtime = 5;
}

guideportcontrollerwidget::~guideportcontrollerwidget() {
	_statusTimer.stop();
	delete ui;
}

void	guideportcontrollerwidget::instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument) {
	// parent setup
	InstrumentWidget::instrumentSetup(serviceobject, instrument);

	// read information about the guideport
	int	index = 0;
	while(_instrument.has(snowstar::InstrumentGuidePort, index)) {
		snowstar::GuidePortPrx	guideport
			= _instrument.guideport(index);
		if (!_guideport) {
			_guideport = guideport;
		}
		std::string	dn = instrument.displayname(
			snowstar::InstrumentGuidePort,
			index, serviceobject.name());
		ui->guideportSelectionBox->addItem(QString(dn.c_str()));
		index++;
	}

	// get the guiderate from the instrument
	if (_instrument.hasProperty("guiderate")) {
		_guiderate = _instrument.doubleProperty("guiderate");
	}
}

void	guideportcontrollerwidget::setupComplete() {
	// set up the guideport
	setupGuideport();
}

void	guideportcontrollerwidget::setupGuideport() {
	_statusTimer.stop();
	if (_guideport) {
		ui->guideWidget->setEnabled(true);
		ui->activationWidget->setEnabled(true);
		_statusTimer.start();
	} else {
		ui->guideWidget->setEnabled(false);
		ui->activationWidget->setEnabled(false);
	}
}

void	guideportcontrollerwidget::guideportChanged(int index) {
	_guideport = _instrument.guideport(index);
	setupGuideport();
	emit guideportSelected(index);
}

void	guideportcontrollerwidget::activateRAplus() {
	if (!_guideport) { return; }
	_guideport->activate(_activationtime, 0);
}

void	guideportcontrollerwidget::activateRAminus() {
	if (!_guideport) { return; }
	_guideport->activate(-_activationtime, 0);
}

void	guideportcontrollerwidget::activateDECplus() {
	if (!_guideport) { return; }
	_guideport->activate(0, _activationtime);
}

void	guideportcontrollerwidget::activateDECminus() {
	if (!_guideport) { return; }
	_guideport->activate(0, -_activationtime);
}

void	guideportcontrollerwidget::setActivationTime(double t) {
	ui->activationtimeSpinBox->setValue(t);
}

void	guideportcontrollerwidget::changeActivationTime(double t) {
	_activationtime = t;
}

void	guideportcontrollerwidget::statusUpdate() {
	if (!_guideport) { return; }
	try {
		unsigned char	newactive = _guideport->active();
		if (newactive != _active) {
			_active = newactive;
			ui->guiderButton->setNorthActive(
				_active & snowstar::DECPLUS);
			ui->guiderButton->setSouthActive(
				_active & snowstar::DECMINUS);
			ui->guiderButton->setWestActive(
				_active & snowstar::RAPLUS);
			ui->guiderButton->setEastActive(
				_active & snowstar::RAMINUS);
			repaint();
		}
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "couldn't get active data: %s",
			x.what());
	}
}

void	guideportcontrollerwidget::radecCorrection(astro::RaDec correction, bool west) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "correction received: %s",
		correction.toString().c_str());

	// convert the correction into a proposed activation of the
	// the guide port pins
	astro::Angle	ra = correction.ra().reduced(-M_PI);
	astro::Angle	dec = correction.dec().reduced(-M_PI);

	// we need the guide rate as an Angle
	astro::Angle	sidereal_rate(2 * M_PI / 86400);
	astro::Angle	omega = sidereal_rate * _guiderate;

	// compute the changes
	float	racorrection = ra.radians() / omega.radians();
	float	deccorrection = ((west) ? 1 : -1) * dec.radians() / omega.radians();

	// propose the activation to the user
	ui->raField->setText(QString(astro::stringprintf("%.1f",
		racorrection).c_str()));
	ui->decField->setText(QString(astro::stringprintf("%.1f",
		deccorrection).c_str()));
}

void	guideportcontrollerwidget::activateClicked() {
	if (!_guideport) { return; }

	// read the activation times from the fields
	bool	ok = true;
	float	racorrection = ui->raField->text().toFloat(&ok);
	if (!ok) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot convert value: %s",
			ui->raField->text().toLatin1().data());
		racorrection = 0;
	}
	float	deccorrection = ui->decField->text().toFloat(&ok);
	if (!ok) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot convert value: %s",
			ui->decField->text().toLatin1().data());
		deccorrection = 0;
	}

	// activate the pins
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"guideport activations: %.3fs,%.3fs",
			racorrection, deccorrection);
		_guideport->activate(racorrection, deccorrection);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot activate %.3f,%.3f: %s",
			racorrection, deccorrection, x.what());
	}
}

} // namespace snowgui
