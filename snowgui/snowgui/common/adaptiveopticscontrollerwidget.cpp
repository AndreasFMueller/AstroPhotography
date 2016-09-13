/*
 * adaptiveopticscontrollerwidget.cpp
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "adaptiveopticscontrollerwidget.h"
#include "ui_adaptiveopticscontrollerwidget.h"
#include <QTimer>

namespace snowgui {

/**
 * \brief create an adaptive optics controller
 */
adaptiveopticscontrollerwidget::adaptiveopticscontrollerwidget(QWidget *parent)
	: InstrumentWidget(parent),
	  ui(new Ui::adaptiveopticscontrollerwidget) {
	ui->setupUi(this);
	ui->adaptiveopticsWidget->setEnabled(false);
	ui->adaptiveopticsSelectionBox->setEnabled(false);

	// connect signals
	connect(ui->adaptiveopticsSelectionBox,
		SIGNAL(currentIndexChanged(int)),
		this, SLOT(adaptiveopticsChanged(int)));
	connect(ui->adaptiveopticsWidget, SIGNAL(pointSelected(QPointF)),
		this, SLOT(setPoint(QPointF)));

	// timer for status updates
	statusTimer.setInterval(100);
	connect(&statusTimer, SIGNAL(timeout()), this, SLOT(statusUpdate()));
}

/**
 * \brief Destroy the adaptive optics controller
 */
adaptiveopticscontrollerwidget::~adaptiveopticscontrollerwidget() {
	delete ui;
	statusTimer.stop();
}

/**
 * \brief add instrument information to the object
 */
void	adaptiveopticscontrollerwidget::instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument) {
	// parent setup
	InstrumentWidget::instrumentSetup(serviceobject, instrument);

	// read the informatio bout the guideport
	int	index = 0;
	while (_instrument.has(snowstar::InstrumentAdaptiveOptics, index)) {
		snowstar::AdaptiveOpticsPrx	adaptiveoptics
			= _instrument.adaptiveoptics(index);
		if (!_adaptiveoptics) {
			_adaptiveoptics = adaptiveoptics;
		}
		ui->adaptiveopticsSelectionBox->addItem(
			QString(adaptiveoptics->getName().c_str()));
		index++;
	}

	// set the adaptive optics unit
	setupAdaptiveOptics();
}

/**
 * \brief read the information about the adaptive optics unit
 */
void	adaptiveopticscontrollerwidget::setupAdaptiveOptics() {
	if (_adaptiveoptics) {
		ui->adaptiveopticsWidget->setEnabled(true);
		ui->adaptiveopticsSelectionBox->setEnabled(true);
		statusTimer.start();
		statusUpdate();
	} else {
		ui->adaptiveopticsWidget->setEnabled(false);
		ui->adaptiveopticsSelectionBox->setEnabled(false);
	}
}

/**
 * \brief The adaptive optics device has changed
 */
void	adaptiveopticscontrollerwidget::adaptiveopticsChanged(int index) {
	_adaptiveoptics = _instrument.adaptiveoptics(index);
	setupAdaptiveOptics();
	emit adaptiveopticsSelected();
}

/**
 * \brief Handle a change of the point
 */
void	adaptiveopticscontrollerwidget::setPoint(QPointF point) {
	snowstar::Point	target;
	target.x = point.x() / 100.;
	target.y = point.y() / 100.;
	try {
		_adaptiveoptics->set(target);
	} catch (...) {
	}
}

/**
 * \brief 
 */
void	adaptiveopticscontrollerwidget::statusUpdate() {
	if (_adaptiveoptics) {
		snowstar::Point	p = _adaptiveoptics->get();
		QPointF	aop(100. * p.x, 100. * p.y);
		ui->adaptiveopticsWidget->setPoint(aop);
	}
}

} // namespace snowgui
