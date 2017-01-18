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

	// set default activation time
	_activationtime = 5;
}

guideportcontrollerwidget::~guideportcontrollerwidget() {
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
		ui->guideportSelectionBox->addItem(
			QString(guideport->getName().c_str()));
		index++;
	}

	// set up the guideport
	setupGuideport();
}

void	guideportcontrollerwidget::setupGuideport() {
	if (_guideport) {
		ui->guideWidget->setEnabled(true);
		ui->activationWidget->setEnabled(true);
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

} // namespace snowgui
