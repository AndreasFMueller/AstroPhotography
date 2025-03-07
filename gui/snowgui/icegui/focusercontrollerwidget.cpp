/*
 * focusercontrollerwidget.cpp -- implementation of focusercontrollerwidget
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "focusercontrollerwidget.h"
#include "ui_focusercontrollerwidget.h"
#include <QTimer>

namespace snowgui {

/**
 * \brief Construct a new focusercontrollerwidget
 */
focusercontrollerwidget::focusercontrollerwidget(QWidget *parent)
	: InstrumentWidget(parent), ui(new Ui::focusercontrollerwidget) {
	ui->setupUi(this);
	ui->currentField->setEnabled(false);
	ui->positionSpinBox->setEnabled(false);
	ui->positionButton->setEnabled(false);

	_backlash_enabled = true;
	ui->backlashCheckBox->setCheckState(Qt::Checked);
	_backlash = 0;

	// GUI element connections
	connect(ui->focuserSelectionBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(focuserChanged(int)));

	connect(ui->positionButton, SIGNAL(clicked()),
		this, SLOT(guiChanged()));
	connect(ui->positionSpinBox, SIGNAL(valueChanged(int)),
		this, SLOT(guiChanged()));
	connect(ui->positionSpinBox, SIGNAL(editingFinished()),
		this, SLOT(editingFinished()));

	connect(ui->backlashCheckBox, SIGNAL(stateChanged(int)),
		this, SLOT(backlashChanged(int)));

	// initialize the timer
	connect(&statusTimer, SIGNAL(timeout()), this, SLOT(statusUpdate()));
	statusTimer.setInterval(100);
}

/**
 * \brief Instrument related setup
 */
void	focusercontrollerwidget::instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument) {
	// parent setup
	InstrumentWidget::instrumentSetup(serviceobject, instrument);

	// read information about Focusers available on this instrument, and 
	// remember the first ccd you can find
	int     index = 0;
	while (_instrument.has(snowstar::InstrumentFocuser, index)) {
		snowstar::FocuserPrx	focuser = _instrument.focuser(index);
		if (!_focuser) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "emitting focuser %d",
				index);
			emit focuserSelected(index);
			_focuser = focuser;
			emit focuserSelected(_focuser);
		}
		std::string	sn = instrument.displayname(
					snowstar::InstrumentFocuser,
					index, serviceobject.name());
		ui->focuserSelectionBox->addItem(QString(sn.c_str()));
		index++;
	}
}

/**
 * \brief Main thread initializations for the focuser
 */
void	focusercontrollerwidget::setupComplete() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setup complete");
	// setup the focuser
	setupFocuser();
}

/**
 * \brief Destroy the focuser controller widget
 */
focusercontrollerwidget::~focusercontrollerwidget() {
	statusTimer.stop();
	delete ui;
}

/**
 * \brief Setup focuser information
 */
void	focusercontrollerwidget::setupFocuser() {
	// make sure the timer does not fire
	statusTimer.stop();

	// make sure no signals are sent
	ui->positionButton->blockSignals(true);
	ui->positionSpinBox->blockSignals(true);

	// read information from the focuser
	if (_focuser) {
		int	minimum = _focuser->min();
		int	maximum = _focuser->max();
		int	current = _focuser->current();
		emit newFocuserPosition(current);
		_backlash = _focuser->backlash();
		if (_backlash == 0) {
			_backlash_enabled = false;
			ui->backlashCheckBox->setCheckState(Qt::Unchecked);
			ui->backlashCheckBox->setEnabled(false);
			ui->backlashField->setText(QString());
			ui->backlashField->setEnabled(false);
		} else {
			_backlash_enabled = true;
			ui->backlashCheckBox->setCheckState(Qt::Checked);
			ui->backlashCheckBox->setEnabled(true);
			ui->backlashField->setText(QString::number(_backlash));
			ui->backlashField->setEnabled(true);
		}
		ui->positionSpinBox->setMaximum(maximum);
		ui->positionSpinBox->setMinimum(minimum);
		ui->positionSpinBox->setValue(current);
		displayCurrent(current);
		ui->positionButton->setEnabled(false);
		statusTimer.start();
		ui->currentField->setEnabled(true);
		ui->positionSpinBox->setEnabled(true);
		ui->positionButton->setEnabled(true);
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no focuser detected");
		ui->currentField->setEnabled(false);
		ui->positionSpinBox->setEnabled(false);
		ui->positionButton->setEnabled(false);
	}

	// now we can release the signals again
	ui->positionButton->blockSignals(false);
	ui->positionSpinBox->blockSignals(false);

	// emit the focuser
	emit focuserSelected(_focuser);
}

/**
 * \brief Get the current focuser position
 */
int	focusercontrollerwidget::getCurrentPosition() {
	try {
		if (_focuser) {
			return _focuser->current();
		}
	} catch (...) {
	}
	return 0;
}

/**
 * \brief Display the current focuser position
 */
void	focusercontrollerwidget::displayCurrent(int current) {
	std::string	currentstring = astro::stringprintf("%d", current);
	ui->currentField->setText(QString(currentstring.c_str()));
}

/**
 * \brief Update the target position info in the GUI
 *
 * This method does not send any signals
 */
void	focusercontrollerwidget::displayTarget(int target) {
	// check that the value is valid 
	int	minimum = _focuser->min();
	int	maximum = _focuser->max();
	int	current = _focuser->current();
	if ((target < minimum) || (target > maximum)) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"position %d not valid: should be between %d and %d",
			target, minimum, maximum);
		// XXX should we throw an exception?
		return;
	}
	ui->positionSpinBox->blockSignals(true);
	ui->positionSpinBox->setValue(target);
	ui->positionSpinBox->blockSignals(false);
	ui->positionButton->setEnabled(current != target);
}

/**
 * \brief This method reads the current position from the focuser
 */
void	focusercontrollerwidget::setCurrent() {
	displayCurrent(_focuser->current());
}

/**
 * \brief This method displays the current target position
 */
void	focusercontrollerwidget::setTarget(int target) {
	displayTarget(target);
}

/**
 * \brief This slot initiates moving to a new target position
 */
void	focusercontrollerwidget::movetoPosition(int target) {
	displayTarget(target);
	startMoving(target);
}

/**
 * \brief 
 */
void	focusercontrollerwidget::startMoving(int target) {
	// we use the delta variable to find out whether we should
	// emit the targetReached signal (later in the statusUpdate slot)
	delta = _focuser->current() - target;
	if ((delta > 0) && (_backlash_enabled)) {
		_backlashing = true;
		target = target - _backlash;
	} else {
		_backlashing = false;
	}
	_focuser->set(target);
}

/**
 * \brief Do it all GUI slot to handle changes to GUI elements
 */
void	focusercontrollerwidget::guiChanged() {
	if (sender() == ui->positionSpinBox) {
		int	current = _focuser->current();
		ui->positionButton->setEnabled(current != ui->positionSpinBox->value());
	}
	if (sender() == ui->positionButton) {
		int	target = ui->positionSpinBox->value();
		startMoving(target);
	}
}

/**
 * \brief Editing the position has finished
 *
 * We handle editing of the target position field differently: if the user
 * chooses to edit the field, then we assume that the result of the edit
 * is the position he wants to move to, so we initiate the move. We cannot
 * do this with value changes, because the user might want to peform many
 * more changes before committing to a new position
 */
void	focusercontrollerwidget::editingFinished() {
	int	target = ui->positionSpinBox->value();
	startMoving(target);
}

/**
 * \brief Timer status update slot
 */
void	focusercontrollerwidget::statusUpdate() {
	if (!_focuser) {
		return;
	}
	int	current = _focuser->current();
	if (current != _previousposition) {
		double	f = current / 5000.;
		ui->positionButton->update(f);
		_previousposition = current;
	}
	displayCurrent(current);
	int	target = ui->positionSpinBox->value();
	bool	targetreached
		= (current == target - ((_backlashing) ? _backlash : 0));
	if ((targetreached) && (_backlashing)) {
		startMoving(target);
		return;
	}
	ui->positionButton->setEnabled(!targetreached);
	if ((targetreached) && (delta != 0)) {
		emit targetPositionReached();
	}
	delta = current - target;
}

/**
 * \brief Slot called when a different focuser is selected
 */
void	focusercontrollerwidget::focuserChanged(int index) {
	statusTimer.stop();
	_focuser = _instrument.focuser(index);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "emitting focuser %d", index);
	emit focuserSelected(index);
	setupFocuser();
}

void	focusercontrollerwidget::backlashChanged(int c) {
	_backlash_enabled = (c > 0);
}

} // namespace snowgui
