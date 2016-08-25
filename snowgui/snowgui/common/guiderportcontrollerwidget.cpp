/*
 * guiderportcontrollerwidget.cpp 
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rappeswil
 */
#include "guiderportcontrollerwidget.h"
#include "ui_guiderportcontrollerwidget.h"
#include <camera.h>

namespace snowgui {

guiderportcontrollerwidget::guiderportcontrollerwidget(QWidget *parent)
	: InstrumentWidget(parent),
	  ui(new Ui::guiderportcontrollerwidget) {
	ui->setupUi(this);
	ui->guideWidget->setEnabled(false);
	ui->activationWidget->setEnabled(false);

	// connect signals
	connect(ui->guiderportSelectionBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(guiderportChanged(int)));

	connect(ui->raplusButton, SIGNAL(clicked()),
		this, SLOT(activateRAplus()));
	connect(ui->raminusButton, SIGNAL(clicked()),
		this, SLOT(activateRAminus()));
	connect(ui->decplusButton, SIGNAL(clicked()),
		this, SLOT(activateDECplus()));
	connect(ui->decminusButton, SIGNAL(clicked()),
		this, SLOT(activateDECminus()));

	connect(ui->activationtimeSpinBox, SIGNAL(valueChanged(double)),
		this, SLOT(changeActivationTime(double)));

	// set default activation time
	_activationtime = 5;
}

guiderportcontrollerwidget::~guiderportcontrollerwidget() {
	delete ui;
}

void	guiderportcontrollerwidget::instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument) {
	// parent setup
	InstrumentWidget::instrumentSetup(serviceobject, instrument);

	// read information about the guiderport
	int	index = 0;
	while(_instrument.has(snowstar::InstrumentGuiderPort, index)) {
		snowstar::GuiderPortPrx	guiderport
			= _instrument.guiderport(index);
		if (!_guiderport) {
			_guiderport = guiderport;
		}
		ui->guiderportSelectionBox->addItem(
			QString(guiderport->getName().c_str()));
		index++;
	}

	// set up the guiderport
	setupGuiderport();
}

void	guiderportcontrollerwidget::setupGuiderport() {
	if (_guiderport) {
		ui->guideWidget->setEnabled(true);
		ui->activationWidget->setEnabled(true);
	} else {
		ui->guideWidget->setEnabled(false);
		ui->activationWidget->setEnabled(false);
	}
}

void	guiderportcontrollerwidget::guiderportChanged(int index) {
	_guiderport = _instrument.guiderport(index);
	setupGuiderport();
	emit guiderportSelected(index);
}

void	guiderportcontrollerwidget::activateRAplus() {
	if (!_guiderport) { return; }
	_guiderport->activate(_activationtime, 0);
}

void	guiderportcontrollerwidget::activateRAminus() {
	if (!_guiderport) { return; }
	_guiderport->activate(-_activationtime, 0);
}

void	guiderportcontrollerwidget::activateDECplus() {
	if (!_guiderport) { return; }
	_guiderport->activate(0, _activationtime);
}

void	guiderportcontrollerwidget::activateDECminus() {
	if (!_guiderport) { return; }
	_guiderport->activate(0, -_activationtime);
}

void	guiderportcontrollerwidget::setActivationTime(double t) {
	ui->activationtimeSpinBox->setValue(t);
}

void	guiderportcontrollerwidget::changeActivationTime(double t) {
	_activationtime = t;
}

} // namespace snowgui
