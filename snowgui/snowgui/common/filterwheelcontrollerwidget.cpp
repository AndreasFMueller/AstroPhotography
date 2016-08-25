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
 * \brief Create a new filterwheelcontrollerwidget
 */
filterwheelcontrollerwidget::filterwheelcontrollerwidget(QWidget *parent)
	: InstrumentWidget(parent), ui(new Ui::filterwheelcontrollerwidget) {
	ui->setupUi(this);
	ui->filterBox->setEnabled(false);

	// connections of GUI components
	connect(ui->filterwheelSelectionBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(filterwheelChanged(int)));
	connect(ui->filterBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(setFilter(int)));

	// initialize the timer
	statusTimer = new QTimer();
	connect(statusTimer, SIGNAL(timeout()), this, SLOT(statusUpdate()));
	statusTimer->setInterval(100);
}

/**
 * \brief Destroy the filterwheelcontrollerwidget
 */
filterwheelcontrollerwidget::~filterwheelcontrollerwidget() {
	statusTimer->stop();
	delete statusTimer;
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
		if (!_filterwheel) {
			_filterwheel = filterwheel;
		}
		ui->filterwheelSelectionBox->addItem(
			QString(filterwheel->getName().c_str()));
		index++;
	}

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
	statusTimer->stop();

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
		} catch (...) {
			ui->filterBox->setEnabled(false);
		}

		// store the current state
		_previousstate = snowstar::FwUNKNOWN;

		// start the timer
		statusTimer->start();
	}
	ui->filterBox->blockSignals(false);
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
	statusTimer->stop();
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
	snowstar::FilterwheelState	newstate = _filterwheel->getState();
	if (newstate != _previousstate) {
		_previousstate = newstate;
		switch (newstate) {
		case snowstar::FwMOVING:
			ui->filterIndicator->start();
			break;
		case snowstar::FwIDLE:
		case snowstar::FwUNKNOWN:
			ui->filterIndicator->stop();
			break;
		}
	}
	switch (newstate) {
	case snowstar::FwIDLE:
		ui->filterBox->setEnabled(true);
		try {
			displayFilter(_filterwheel->currentPosition());
		} catch (...) { }
		break;
	case snowstar::FwMOVING:
	case snowstar::FwUNKNOWN:
		ui->filterBox->setEnabled(false);
		break;
	}
}

} // namespace snowgui
