/*
 * filterwheelcontrollerwidget.cpp -- Implementation of Filterwheel controller
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "filterwheelcontrollerwidget.h"
#include "ui_filterwheelcontrollerwidget.h"
#include <camera.h>
#include <QTimer>

namespace snowgui {

/**
 * \brief Filterwheel update thread
 *
 * This is just a slot to call the status update in the filterwheel instance.
 * This is not a method of the filterwheelcontrollerwidget, but since this
 * is the only method of the update thread class, it does not make sense
 * to put it into a separate file.
 */
void    filterwheelupdatethread::statusUpdate() {
	if (_filterwheelcontrollerwidget) {
		_filterwheelcontrollerwidget->statusUpdate();
	}
}

void    filterwheelupdatethread::positionUpdate() {
	if (_filterwheelcontrollerwidget) {
		_filterwheelcontrollerwidget->positionUpdate();
	}
}

/**
 * \brief Create a new filterwheelcontrollerwidget
 */
filterwheelcontrollerwidget::filterwheelcontrollerwidget(QWidget *parent)
	: InstrumentWidget(parent), ui(new Ui::filterwheelcontrollerwidget) {
	ui->setupUi(this);
	ui->filterBox->setEnabled(false);
	ui->filterIndicator->setEnabled(false);

	// connections of GUI components
	connect(ui->filterwheelSelectionBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(filterwheelChanged(int)));
	connect(ui->filterBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(setFilter(int)));

	// connect start/stop signals
	connect(this, SIGNAL(filterwheelStart()),
		ui->filterIndicator, SLOT(start()));
	connect(this, SIGNAL(filterwheelStop()),
		ui->filterIndicator, SLOT(stop()));
	connect(this,
		SIGNAL(filterwheelStateChanged(snowstar::FilterwheelState)),
		this, SLOT(filterwheelNewState(snowstar::FilterwheelState)));
	connect(this, SIGNAL(filterwheelPositionChanged(int)),
		this, SLOT(filterwheelNewPosition(int)));

	// initialize the timer
	_updatethread = new filterwheelupdatethread(this);
	connect(&statusTimer, SIGNAL(timeout()),
		_updatethread, SLOT(statusUpdate()));
	connect(&statusTimer, SIGNAL(timeout()),
		_updatethread, SLOT(positionUpdate()));
	statusTimer.setInterval(100);
}

/**
 * \brief Destroy the filterwheelcontrollerwidget
 */
filterwheelcontrollerwidget::~filterwheelcontrollerwidget() {
	statusTimer.stop();
	delete _updatethread;
	delete ui;
}

/**
 * \brief Common instrument setup
 */
void	filterwheelcontrollerwidget::instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument) {
	// parent setup
	InstrumentWidget::instrumentSetup(serviceobject, instrument);

	// read the information about filterwheels on this instrumetn, and
	// remember the first cooler you can find
	int	index = 0;
	while(_instrument.has(snowstar::InstrumentFilterWheel, index)) {
		snowstar::FilterWheelPrx	filterwheel
			= _instrument.filterwheel(index);
		std::string	sn = instrument.displayname(
					snowstar::InstrumentFilterWheel,
					index, serviceobject.name());
		ui->filterwheelSelectionBox->addItem(QString(sn.c_str()));

		// query the filterwheel before making it generally available
		// this is supposed to prevent clients from asking the
		// filterwheel anything before it is ready
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"current filterwheel position: %d",
			filterwheel->currentPosition());

		// by setting the filterwheel proxy, the filter wheel becomes
		// generally available
		if (!_filterwheel) {
			_filterwheel = filterwheel;
		}
		index++;
	}

	// ask for the current position (because that may take a long
	// time initially
}

/**
 * \brief main thread stuff for filterwheel initialization
 */
void	filterwheelcontrollerwidget::setupComplete() {
	// set the filterwheel up
	setupFilterwheel();
}

/**
 * \brief Setup the filterwheel
 *
 * This method is called each time a new filterwheel is selected.
 * It reads the relevant information about the filterwheeel from the
 * remote server and initializes the GUI elements with it.
 */
void	filterwheelcontrollerwidget::setupFilterwheel() {
	ui->filterBox->blockSignals(true);

	// make sure the status timer does not fire
	statusTimer.stop();

	// remove previous content of the filterwheel
	while (ui->filterBox->count() > 0) {
		ui->filterBox->removeItem(0);
	}

	// get information about the filterwheel
	if (_filterwheel) {
		// add filter names
		int	nfilters = _filterwheel->nFilters();
		for (int i = 0; i < nfilters; i++) {
			std::string	name = _filterwheel->filterName(i);
			QString	qname(name.c_str());
			ui->filterBox->addItem(qname);
		}

		// set the current position
		try {
			int	pos = _filterwheel->currentPosition();
			ui->filterIndicator->position(pos);
			ui->filterBox->setCurrentIndex(pos);
			ui->filterBox->setEnabled(true);
			ui->filterIndicator->setEnabled(true);
		} catch (...) {
			ui->filterBox->setEnabled(false);
			ui->filterIndicator->setEnabled(false);
		}

		// store the current state
		_previousstate = snowstar::FwUNKNOWN;

		// start the timer
		statusTimer.start();
	}
	ui->filterBox->blockSignals(false);

	// emit the filterwheel selected signal
	emit filterwheelSelected(_filterwheel);
	emit filterwheelSelected(ui->filterwheelSelectionBox->currentIndex());
}

/**
 * \brief display the modified filter selection
 *
 * This method does not send any signals
 */
void	filterwheelcontrollerwidget::displayFilter(int index) {
	ui->filterBox->blockSignals(true);
	ui->filterBox->setCurrentIndex(index);
	ui->filterBox->blockSignals(false);
}

/**
 * \brief Slot to change the filter 
 */
void    filterwheelcontrollerwidget::setFilter(int index) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setFilter(%d)", index);
	_filterwheel->select(index);
}

/**
 * \brief Change the filter wheel
 *
 * This slot is activate when the user chooses a different filter wheel.
 */
void    filterwheelcontrollerwidget::filterwheelChanged(int index) {
	statusTimer.stop();
	_filterwheel = _instrument.filterwheel(index);
	setupFilterwheel();
}

/**
 * \brief Slot for timer status update
 *
 * This slot is connected to the timeout() signal of the status timer.
 */
void    filterwheelcontrollerwidget::statusUpdate() {
	if (!_filterwheel) {
		return;
	}

	// get the new state
	snowstar::FilterwheelState	newstate;
	try {
		newstate = _filterwheel->getState();
	} catch (...) {
		return;
	}

	// if the state has not changed, don't do anything
	if (newstate == _previousstate) {
		return;
	}

	// emit filter wheel busy indicator signals if necessary
	_previousstate = newstate;
	switch (newstate) {
	case snowstar::FwMOVING:
		emit filterwheelStart();
		break;
	case snowstar::FwIDLE:
	case snowstar::FwUNKNOWN:
		emit filterwheelStop();
		break;
	}

	// emit the signal that indicates the new state
	emit filterwheelStateChanged(newstate);
}

void	filterwheelcontrollerwidget::positionUpdate() {
	if (!_filterwheel) {
		return;
	}

	try {
		int	pos = _filterwheel->currentPosition();
		if (pos != _position) {
			emit filterwheelPositionChanged(pos);
		}
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"cannot get filterwheel position: %s", x.what());
	}
}

/**
 * \brief Changes to the user interface 
 *
 * This slot has to be executed on the main ghread
 */
void	filterwheelcontrollerwidget::filterwheelNewState(
		snowstar::FilterwheelState newstate) {
	// update user interface elements
	switch (newstate) {
	case snowstar::FwIDLE:
		ui->filterBox->setEnabled(true);
		ui->filterIndicator->setEnabled(true);
		break;
	case snowstar::FwMOVING:
	case snowstar::FwUNKNOWN:
		ui->filterBox->setEnabled(false);
		break;
	}
}

void	filterwheelcontrollerwidget::filterwheelNewPosition(int position) {
	_position = position;
	// update the menu position
}

} // namespace snowgui
