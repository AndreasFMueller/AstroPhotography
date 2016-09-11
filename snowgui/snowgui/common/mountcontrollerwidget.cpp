/*
 * mountcontrollerwidget.cpp
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "mountcontrollerwidget.h"
#include "ui_mountcontrollerwidget.h"
#include <QMessageBox>

namespace snowgui {

/**
 * \brief Create a new mount controller widget
 */
mountcontrollerwidget::mountcontrollerwidget(QWidget *parent)
	: InstrumentWidget(parent),
	  ui(new Ui::mountcontrollerwidget) {
	ui->setupUi(this);

	_previousstate = snowstar::MountIDLE;

	connect(ui->gotoButton, SIGNAL(clicked()),
		this, SLOT(gotoClicked()));

	_statusTimer.setInterval(1000);

	connect(&_statusTimer, SIGNAL(timeout()),
		this, SLOT(statusUpdate()));
}

/**
 * \brief Destroy the mount controller widget
 */
mountcontrollerwidget::~mountcontrollerwidget() {
	_statusTimer.stop();
	delete ui;
}

/**
 * \brief Setup the instrument
 */
void	mountcontrollerwidget::instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument) {
	// parent setup
	InstrumentWidget::instrumentSetup(serviceobject, instrument);

	// read information about mounts
	int	index = 0;
	while (_instrument.has(snowstar::InstrumentMount, index)) {
		snowstar::MountPrx	mount = _instrument.mount(index);
		if (!_mount) {
			_mount = mount;
		}
		ui->mountSelectionBox->addItem(
			QString(mount->getName().c_str()));
		index++;
	}

	// setup the mount
	setupMount();
}

/**
 * \brief setup the mount
 */
void	mountcontrollerwidget::setupMount() {
	_statusTimer.stop();
	_previousstate = snowstar::MountIDLE;
	if (_mount) {
		ui->raField->setEnabled(true);
		ui->decField->setEnabled(true);
		ui->gotoButton->setEnabled(true);
	} else {
		ui->raField->setEnabled(true);
		ui->decField->setEnabled(true);
		ui->gotoButton->setEnabled(true);
		ui->gotoButton->setText(QString("GOTO"));
		ui->currentField->setText(QString("(idle)"));
	}
	_statusTimer.start();
}

static QString	rangemessage("The RA value must be between 0 and 24 hours, "
			"and the DEC value must be between -90 and -90°");

/**
 * \brief What to do when the user clicks the goto button
 */
void	mountcontrollerwidget::gotoClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "goto clicked");
	switch (_previousstate) {
	case snowstar::MountGOTO:
		_mount->cancel();
		return;
	default:
		break;
	}
	snowstar::RaDec	radec;
	radec.ra = ui->raField->text().toDouble();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found RA = %.4f", radec.ra);
	if ((radec.ra < 0) || (radec.ra > 24)) {
		QMessageBox	message(this);
		message.setText(QString("Invalid RA"));
		message.setInformativeText(rangemessage);
		message.exec();
		return;
	}
	radec.dec = ui->decField->text().toDouble();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found DEC = %.4f", radec.dec);
	if ((radec.dec < -90) || (radec.dec > 90)) {
		QMessageBox	message(this);
		message.setText(QString("Invalid DEC"));
		message.setInformativeText(rangemessage);
		message.exec();
		return;
	}
	_mount->GotoRaDec(radec);
}

/**
 * \brief Slot called when the timer expires
 */
void	mountcontrollerwidget::statusUpdate() {
	snowstar::mountstate state = _mount->state();
	if (state != _previousstate) {
		_previousstate = state;
		switch (state) {
		case snowstar::MountIDLE:
			ui->currentField->setText(QString("(idle)"));
			ui->gotoButton->setText(QString("GOTO"));
			ui->gotoButton->setEnabled(false);
			return;
		case snowstar::MountALIGNED:
			ui->currentField->setText(QString("(aligned)"));
			ui->gotoButton->setText(QString("GOTO"));
			ui->gotoButton->setEnabled(true);
			return;
		case snowstar::MountTRACKING:
			ui->gotoButton->setText(QString("GOTO"));
			ui->gotoButton->setEnabled(true);
			break;
		case snowstar::MountGOTO:
			ui->gotoButton->setText(QString("Cancel"));
			ui->gotoButton->setEnabled(true);
			break;
		}
	}
	snowstar::RaDec	radec = _mount->getRaDec();
	std::string	s = astro::stringprintf("RA: %.4f, DEC: %.4f",
		radec.ra, radec.dec);
	ui->currentField->setText(QString(s.c_str()));
}

/**
 * \brief Slot called when the selection of the mount changes
 */
void	mountcontrollerwidget::mountChanged(int index) {
	_mount = _instrument.mount(index);
	setupMount();
	emit mountSelected(index);
}

} // namespace snowgui
