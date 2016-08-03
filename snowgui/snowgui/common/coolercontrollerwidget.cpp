/*
 * coolercontrollerwidget.cpp -- Cooler controller implementation
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "coolercontrollerwidget.h"
#include "ui_coolercontrollerwidget.h"
#include <QTimer>

namespace snowgui {

coolercontrollerwidget::coolercontrollerwidget(QWidget *parent) :
	InstrumentWidget(parent), ui(new Ui::coolercontrollerwidget) {
	ui->setupUi(this);
	statusTimer = NULL;
}

void	coolercontrollerwidget::instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument) {
	// parent setup
	InstrumentWidget::instrumentSetup(serviceobject, instrument);

	// read information about Coolers on this instrument, and
	// remember the first cooler you can find
	int	index = 0;
	while (_instrument.has(snowstar::InstrumentCooler, index)) {
		snowstar::CoolerPrx	cooler = _instrument.cooler(index);
		if (!_cooler) {
			_cooler = cooler;
		}
		ui->coolerSelectionBox->addItem(
			QString(cooler->getName().c_str()));
		index++;
	}

	// initialize the timer
#if 0
	statusTimer = new QTimer();
	connect(statusTimer, SIGNAL(timeout()), this, SLOT(statusUpdate()));
	statusTimer->setInterval(100);
#endif

	// set the cooler
	setupCooler();
}

coolercontrollerwidget::~coolercontrollerwidget() {
	if (statusTimer) {
		statusTimer->stop();
		delete statusTimer;
	}
	delete ui;
}

void	coolercontrollerwidget::setupCooler() {
	ui->setTemperatureSpinBox->blockSignals(true);
	ui->onoffBox->blockSignals(true);
	if (_cooler) {
		float	actual = _cooler->getActualTemperature() - 273.15;
		QString	actualstring(
			astro::stringprintf("%.1f", actual).c_str());
		ui->actualTemperatureField->setText(actualstring);

		ui->setTemperatureSpinBox->setValue(
			_cooler->getSetTemperature() - 273.15);

		ui->onoffBox->setChecked(_cooler->isOn());
		if (statusTimer) {
			statusTimer->start();
		}
	}
	ui->setTemperatureSpinBox->blockSignals(false);
	ui->onoffBox->blockSignals(false);
}

void	coolercontrollerwidget::displayActualTemperature(float actual) {
	std::string	actualstring = astro::stringprintf("%.1f", actual);
	ui->actualTemperatureField->setText(QString(actualstring.c_str()));
}

void	coolercontrollerwidget::setActual() {
	displayActualTemperature(_cooler->getActualTemperature() - 273.15);
}

void	coolercontrollerwidget::displaySetTemperature(float settemp) {
	if ((settemp < -50) && (settemp > 50)) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"temperature %.1f invalid, not between -50 and 50",
			settemp);
		return;
	}
	float	actual = _cooler->getActualTemperature() - 273.15;
	ui->setTemperatureSpinBox->blockSignals(true);
	ui->setTemperatureSpinBox->setValue(settemp + 273.15);
	ui->setTemperatureSpinBox->blockSignals(false);
	ui->setTemperatureButton->setChecked(actual != settemp);
}

void	coolercontrollerwidget::setSetTemperature(double t) {
	displaySetTemperature(t);
}

void	coolercontrollerwidget::statusUpdate() {
	if (!_cooler) {
		return;
	}
	float	actual = _cooler->getActualTemperature() - 273.15;
	float	settemp = _cooler->getSetTemperature() - 273.15;
	bool	actualreached = (actual == settemp);
	displayActualTemperature(actual);
	if (actualreached) {
		emit setTemperatureReached();
	}
}

void	coolercontrollerwidget::guiChanged() {
	if (sender() == ui->onoffBox) {
		_cooler->setOn(ui->onoffBox->isChecked());
	}
	if (sender() == ui->setTemperatureSpinBox) {
	}
	if (sender() == ui->setTemperatureButton) {
		_cooler->setTemperature(
			ui->setTemperatureSpinBox->value() + 273.15);
	}
}

void	coolercontrollerwidget::coolerChanged(int index) {
	if (statusTimer) {
		statusTimer->stop();
	}
	_cooler = _instrument.cooler(index);
	setupCooler();
}

void	coolercontrollerwidget::editingFinished() {
	_cooler->setTemperature(ui->setTemperatureSpinBox->value() + 273.15);
}

} // namespace snowgui
