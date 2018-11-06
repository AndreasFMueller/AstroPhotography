/*
 * guideportcontrollerwidget.cpp 
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rappeswil
 */
#include "guideportcontrollerwidget.h"
#include "ui_guideportcontrollerwidget.h"
#include <camera.h>

namespace snowgui {

/**
 * \brief Construct a Guideport controller
 *
 * \param parent	the parent widget
 */
guideportcontrollerwidget::guideportcontrollerwidget(QWidget *parent)
	: InstrumentWidget(parent),
	  ui(new Ui::guideportcontrollerwidget) {
	ui->setupUi(this);
	ui->guideWidget->setEnabled(false);
	ui->activationWidget->setEnabled(false);
	ui->proposalWidget->setEnabled(false);

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

/**
 * \brief Destroy the guideport controller
 */
guideportcontrollerwidget::~guideportcontrollerwidget() {
	_statusTimer.stop();
	delete ui;
}

/**
 * \brief Setup the instrument comonents
 */
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

/**
 * \brief Slot called when the guiderpot instrument setup is complete
 */
void	guideportcontrollerwidget::setupComplete() {
	// set up the guideport
	setupGuideport();
}

/**
 * \brief GUI components setup
 */
void	guideportcontrollerwidget::setupGuideport() {
	_statusTimer.stop();
	if (_guideport) {
		try {
			// try to 
			_guideport->active();
			_statusTimer.start();
		} catch (const std::exception& x) {
			std::string	msg = astro::stringprintf("cannot "
				"connect to '%s'", instrumentname().c_str());
			return;
		}
		ui->guideWidget->setEnabled(true);
		ui->activationWidget->setEnabled(true);
		ui->proposalWidget->setEnabled(true);
	} else {
		ui->guideWidget->setEnabled(false);
		ui->activationWidget->setEnabled(false);
		ui->proposalWidget->setEnabled(false);
	}
}

/**
 * \brief Slot called when a different guide port is selected
 */
void	guideportcontrollerwidget::guideportChanged(int index) {
	_guideport = _instrument.guideport(index);
	setupGuideport();
	emit guideportSelected(index);
}

/**
 * \brief Slot called when the user presses RA+
 */
void	guideportcontrollerwidget::activateRAplus() {
	if (!_guideport) { return; }
	_guideport->activate(_activationtime, 0);
}

/**
 * \brief Slot called when the user presses RA-
 */
void	guideportcontrollerwidget::activateRAminus() {
	if (!_guideport) { return; }
	_guideport->activate(-_activationtime, 0);
}

/**
 * \brief Slot called when the user presses DEC+
 */
void	guideportcontrollerwidget::activateDECplus() {
	if (!_guideport) { return; }
	_guideport->activate(0, _activationtime);
}

/**
 * \brief Slot called when the user presses DEC-
 */
void	guideportcontrollerwidget::activateDECminus() {
	if (!_guideport) { return; }
	_guideport->activate(0, -_activationtime);
}

/**
 * \brief Slot used to change the activation time display 
 */
void	guideportcontrollerwidget::setActivationTime(double t) {
	ui->activationtimeSpinBox->setValue(t);
}

/**
 * \brief Slot alled when the user changes the activation time
 */
void	guideportcontrollerwidget::changeActivationTime(double t) {
	_activationtime = t;
}

/**
 * \brief Slot called to update the status of the guide port
 *
 * This slot is activated by the timer at regular intervals
 */
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

/**
 * \brief Slot called to compute the RA/DEC correction from an image point
 *
 * This slot computes the RA/DEC correction to correctly point the telesceop
 * via a guideport correction
 */
void	guideportcontrollerwidget::radecCorrection(astro::RaDec correction,
		bool west) {
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
	int	sign = (west) ? 1 : -1;
	float	racorrection = 0.5 * ra.radians() / omega.radians();
	float	deccorrection = -0.5 * sign * dec.radians() / omega.radians();

	// propose the activation to the user
	ui->raField->setText(QString(astro::stringprintf("%.1f",
		racorrection).c_str()));
	ui->decField->setText(QString(astro::stringprintf("%.1f",
		deccorrection).c_str()));
}

/**
 * \brief Slot to perform the suggested RA/DEC correction
 *
 * This slot reads the activation times including their sign from the
 * textfields and applies them to the guide port
 */
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
