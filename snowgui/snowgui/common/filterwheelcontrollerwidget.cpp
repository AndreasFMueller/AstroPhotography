/*
 * filterwheelcontrollerwidget.cpp
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "filterwheelcontrollerwidget.h"
#include "ui_filterwheelcontrollerwidget.h"
#include <camera.h>
#include <QTimer>

namespace snowgui {

filterwheelcontrollerwidget::filterwheelcontrollerwidget(QWidget *parent)
	: InstrumentWidget(parent), ui(new Ui::filterwheelcontrollerwidget) {
	    ui->setupUi(this);
	statusTimer = NULL;
}

filterwheelcontrollerwidget::~filterwheelcontrollerwidget() {
	if (statusTimer) {
		statusTimer->stop();
		delete statusTimer;
	}
	delete ui;
}

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

	// initialize the timer
	statusTimer = new QTimer();
	connect(statusTimer, SIGNAL(timeout()), this, SLOT(statusUpdate()));
	statusTimer->setInterval(100);

	// set the filterwheel up
	setupFilterwheel();
}

void	filterwheelcontrollerwidget::setupFilterwheel() {
	ui->filterBox->blockSignals(true);

	// remove previous content of the filterwheel
	while (ui->filterBox->count() > 0) {
		ui->filterBox->removeItem(0);
	}

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
			ui->filterBox->setCurrentIndex(
				_filterwheel->currentPosition());
			ui->filterBox->setEnabled(true);
		} catch (...) {
			ui->filterBox->setEnabled(false);
		}

		// start the timer
		if (statusTimer) {
			statusTimer->start();
		}
	}
	ui->filterBox->blockSignals(false);
}

void	filterwheelcontrollerwidget::displayFilter(int index) {
	ui->filterBox->blockSignals(true);
	ui->filterBox->setCurrentIndex(index);
	ui->filterBox->blockSignals(false);
}

void    filterwheelcontrollerwidget::setFilter(int index) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setFilter(%d)", index);
	_filterwheel->select(index);
}

void    filterwheelcontrollerwidget::guiChanged() {
	try {
		_filterwheel->select(ui->filterBox->currentIndex());
	} catch (...) {
	}
}

void    filterwheelcontrollerwidget::filterwheelChanged(int index) {
	if (statusTimer) {
		statusTimer->stop();
	}
	_filterwheel = _instrument.filterwheel(index);
	setupFilterwheel();
}

void    filterwheelcontrollerwidget::statusUpdate() {
	if (!_filterwheel) {
		return;
	}
	switch (_filterwheel->getState()) {
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
