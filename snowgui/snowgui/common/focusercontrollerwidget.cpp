/*
 * focusercontrollerwidget.cpp -- implementation of focusercontrollerwidget
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "focusercontrollerwidget.h"
#include "ui_focusercontrollerwidget.h"
#include <QTimer>

namespace snowgui {

focusercontrollerwidget::focusercontrollerwidget(QWidget *parent)
	: InstrumentWidget(parent), ui(new Ui::focusercontrollerwidget) {
	ui->setupUi(this);
	statusTimer = NULL;
}

void	focusercontrollerwidget::instrumentSetup(
	astro::discover::ServiceObject serviceobject,
	snowstar::RemoteInstrument instrument) {
	// parent setup
	InstrumentWidget::instrumentSetup(serviceobject, instrument);

	// read information about CCDs available on this instrument, and 
	// remember the first ccd you can find
	int     index = 0;
	while (_instrument.has(snowstar::InstrumentFocuser, index)) {
		snowstar::FocuserPrx	focuser = _instrument.focuser(index);
		if (!_focuser) {
			_focuser = focuser;
		}
		ui->focuserSelectionBox->addItem(
			QString(focuser->getName().c_str()));
		index++;
	}

	// initialize the timer
	statusTimer = new QTimer();
	connect(statusTimer, SIGNAL(timeout()), this, SLOT(statusUpdate()));
	statusTimer->setInterval(100);

	// setup the focuser
	setupFocuser();
}

focusercontrollerwidget::~focusercontrollerwidget() {
	if (statusTimer) {
		statusTimer->stop();
		delete statusTimer;
	}
	delete ui;
}

void	focusercontrollerwidget::setupFocuser() {
	ui->positionButton->blockSignals(true);
	ui->positionSpinBox->blockSignals(true);

	if (_focuser) {
		int	minimum = _focuser->min();
		int	maximum = _focuser->max();
		int	current = _focuser->current();
		ui->positionSpinBox->setMaximum(maximum);
		ui->positionSpinBox->setMinimum(minimum);
		ui->positionSpinBox->setValue(current);
		displayCurrent(current);
		ui->positionButton->setEnabled(false);
		if (statusTimer) {
			statusTimer->start();
		}
	}

	ui->positionButton->blockSignals(false);
	ui->positionSpinBox->blockSignals(false);
}

void	focusercontrollerwidget::displayCurrent(int current) {
	std::string	currentstring = astro::stringprintf("%d", current);
	ui->currentField->setText(QString(currentstring.c_str()));
}

void	focusercontrollerwidget::displayTarget(int target) {
	// check that the value is valid 
	int	minimum = _focuser->min();
	int	maximum = _focuser->max();
	int	current = _focuser->current();
	if ((target < minimum) || (target > maximum)) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"position %d not valid: should be between %d and %d",
			target, minimum, maximum);
		// XXX should we throw an exception
		return;
	}
	ui->positionSpinBox->blockSignals(true);
	ui->positionSpinBox->setValue(target);
	ui->positionSpinBox->blockSignals(false);
	ui->positionButton->setEnabled(current != target);
}

void	focusercontrollerwidget::setCurrent() {
	displayCurrent(_focuser->current());
}

void	focusercontrollerwidget::setTarget(int target) {
	displayTarget(target);
}

void	focusercontrollerwidget::guiChanged() {
	if (sender() == ui->positionSpinBox) {
		int	current = _focuser->current();
		ui->positionButton->setEnabled(current != ui->positionSpinBox->value());
	}
	if (sender() == ui->positionButton) {
		_focuser->set(ui->positionSpinBox->value());
	}
}

void	focusercontrollerwidget::editingFinished() {
	_focuser->set(ui->positionSpinBox->value());
}

void	focusercontrollerwidget::statusUpdate() {
	if (!_focuser) {
		return;
	}
	int	current = _focuser->current();
	bool	targetreached = current == ui->positionSpinBox->value();
	ui->positionButton->setEnabled(!targetreached);
	displayCurrent(current);
	if (targetreached) {
		emit targetPositionReached();
	}
}

void	focusercontrollerwidget::focuserChanged(int index) {
	if (statusTimer) {
		statusTimer->stop();
	}
	_focuser = _instrument.focuser(index);
	setupFocuser();
}

} // namespace snowgui
