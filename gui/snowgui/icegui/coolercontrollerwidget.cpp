/*
 * coolercontrollerwidget.cpp -- Cooler controller implementation
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "coolercontrollerwidget.h"
#include "ui_coolercontrollerwidget.h"
#include <QTimer>
#include <sstream>
#include <QMessageBox>

namespace snowgui {

/**
 * \brief Create a coolercontroller widget
 */
coolercontrollerwidget::coolercontrollerwidget(QWidget *parent) :
	InstrumentWidget(parent), ui(new Ui::coolercontrollerwidget) {
	ui->setupUi(this);
	ui->actualTemperatureField->setEnabled(false);
	ui->setTemperatureSpinBox->setEnabled(false);
	ui->activeWidget->setEnabled(false);
	ui->activeWidget->setValue(1);

	// connect signals
	connect(ui->coolerSelectionBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(coolerChanged(int)));

	connect(ui->setTemperatureSpinBox, SIGNAL(valueChanged(double)),
		this, SLOT(guiChanged()));
	connect(ui->setTemperatureSpinBox, SIGNAL(editingFinished()),
		this, SLOT(editingFinished()));
	connect(ui->activeWidget, SIGNAL(toggled(bool)),
		this, SLOT(activeToggled(bool)));

	// updating the active display from information read from the proxy
	connect(this, SIGNAL(newCoolerState(float,float,bool)),
		ui->activeWidget, SIGNAL(update(float,float,bool)));
	connect(this, SIGNAL(newActualTemperature(float)),
		this, SLOT(displayActualTemperature(float)));

	_updatethread = new coolerupdatethread(this);
	_updatethread->moveToThread(_updatethread);
	connect(_updatethread, SIGNAL(finished()),
		_updatethread, SLOT(deleteLater()));

	// initialize the timer
	connect(&statusTimer, SIGNAL(timeout()),
		_updatethread, SLOT(statusUpdate()));
	_updatethread->start();

	statusTimer.setInterval(1000);
}

/**
 * \brief Set up the cooler controller widget with an instrument
 */
void	coolercontrollerwidget::instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument) {
	// parent setup
	InstrumentWidget::instrumentSetup(serviceobject, instrument);

	// read information about Coolers on this instrument, and
	// remember the first cooler you can find
	int	index = 0;
	while (_instrument.has(snowstar::InstrumentCooler, index)) {
		try {
			snowstar::CoolerPrx	cooler = _instrument.cooler(index);
			std::string	sn = _instrument.displayname(
					snowstar::InstrumentCooler, index,
					serviceobject.name());
			_cooler_names.push_back(sn);
			if (!_cooler) {
				_cooler = cooler;
				emit coolerSelected(index);
			}
		} catch (const std::exception& x) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "ignore cooler %d", index);
		}
		index++;
	}
}

/**
 * \brief main thread initializations
 */
void	coolercontrollerwidget::setupComplete() {
	// add the cooler names
	std::vector<std::string>::const_iterator	i;
	for (i = _cooler_names.begin(); i != _cooler_names.end(); i++) {
		ui->coolerSelectionBox->addItem(QString(i->c_str()));
	}

	// set the cooler
	setupCooler();
}

/**
 * \brief Destroy the cooler controller widget
 */
coolercontrollerwidget::~coolercontrollerwidget() {
	statusTimer.stop();
	_updatethread->stop();
	_updatethread->quit();
	delete ui;
}

/**
 * \brief Set up the cooler
 */
void	coolercontrollerwidget::setupCooler() {
	ui->setTemperatureSpinBox->blockSignals(true);
	if (_cooler) {
		// enable all input widgets
		ui->actualTemperatureField->setEnabled(true);
		ui->setTemperatureSpinBox->setEnabled(true);
		ui->activeWidget->setEnabled(true);

		// display the actual temperature
		float	actual = 0;
		float	settemperature = 0;
		bool	ison = false;
		try {
			actual = _cooler->getActualTemperature() - 273.15;
			settemperature = _cooler->getSetTemperature() - 273.15;
			ison = _cooler->isOn();
		} catch (const std::exception& x) {
			coolerFailed(x);
		}
		QString	actualstring(
			astro::stringprintf("%.1f", actual).c_str());
		ui->actualTemperatureField->setText(actualstring);

		// display the set temperature
		ui->setTemperatureSpinBox->setValue(settemperature);

		// display whether the cooler is on
		ui->activeWidget->setActive(ison);

		// enable the status update timer
		statusTimer.start();
	} else {
		// with no cooler, we just stay at temperature 1
		ui->activeWidget->setValue(1);
	}
	ui->setTemperatureSpinBox->blockSignals(false);
}

/**
 * \brief Display an error message if we cannot talk to the cooler
 */
void	coolercontrollerwidget::coolerFailed(const std::exception& x) {
	_cooler = NULL;
	ui->actualTemperatureField->setEnabled(false);
	ui->setTemperatureSpinBox->setEnabled(false);
	ui->activeWidget->setEnabled(false);
	QMessageBox	message;
	message.setText(QString(""));
	std::ostringstream	out;
	out << "Communcation with the cooler '";
	out << ui->coolerSelectionBox->currentText().toLatin1().data();
	out << "' failed: ";
	out << x.what();
	out << ". ";
	out << "The connection has been dropped, the cooler can no longer be used.";
	message.setInformativeText(QString(out.str().c_str()));
	message.exec();
}

/**
 * \brief display the actual temperature
 */
void	coolercontrollerwidget::displayActualTemperature(float actual) {
	std::string	actualstring = astro::stringprintf("%.1f", actual);
	ui->actualTemperatureField->setText(QString(actualstring.c_str()));
}

/**
 * \brief Set the actual temperature 
 */
void	coolercontrollerwidget::setActual() {
	if (_cooler) {
		displayActualTemperature(_cooler->getActualTemperature() - 273.15);
	}
}

/**
 * \brief Display the set temperature
 *
 * \param settemp	set temperature in degrees Celsius
 */
void	coolercontrollerwidget::displaySetTemperature(float settemp) {
	if ((settemp < -50) && (settemp > 50)) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"temperature %.1f invalid, not between -50 and 50",
			settemp);
		return;
	}
	ui->setTemperatureSpinBox->blockSignals(true);
	ui->setTemperatureSpinBox->setValue(settemp + 273.15);
	ui->setTemperatureSpinBox->blockSignals(false);
}

/**
 * \brief Set the set temperature
 *
 *
 * \param t	temperature in degrees Celsius
 */
void	coolercontrollerwidget::setSetTemperature(double t) {
	displaySetTemperature(t);
}

/**
 * \brief Slot for timer updates
 */
void	coolercontrollerwidget::statusUpdate() {
	if (!_cooler) {
		return;
	}
	try {
		// get the information about the cooler
		float	actual = _cooler->getActualTemperature() - 273.15;
		float	settemp = _cooler->getSetTemperature() - 273.15;
		bool	is_on = _cooler->isOn();

		// emit the information to the active
		emit newCoolerState(actual, settemp, is_on);

		// emit the information about the new actual temperature
		emit newActualTemperature(actual);

		// have we already reached the temperature?
		bool	actualreached = (actual == settemp);
		if (actualreached) {
			emit setTemperatureReached();
		}
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot talk to cooler: %s",
			x.what());
	}
}

/**
 * \brief Slot called when the gui changes
 */
void	coolercontrollerwidget::guiChanged() {
	if (sender() == ui->setTemperatureSpinBox) {
		double	t = ui->setTemperatureSpinBox->value();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "temperature changed to %f", t);
		sendSetTemperature(t);
	}
}

/**
 * \brief Handle selection of a new cooler
 */
void	coolercontrollerwidget::coolerChanged(int index) {
	statusTimer.stop();
	try {
		_cooler = _instrument.cooler(index);
		emit coolerSelected(index);
	} catch (const std::exception& x) {
		coolerFailed(x);
	}
	setupCooler();
}

/**
 * \brief When the temperature value has changed
 */
void	coolercontrollerwidget::editingFinished() {
	double	temp = ui->setTemperatureSpinBox->value();
	sendSetTemperature(temp);
}

/**
 * \brief send the set temperature to the server
 *
 * If setting the temperature fails, we read the current set temperature
 * and write that to the spinbox
 *
 * \param temp	temperature in degress Celsius
 */
void	coolercontrollerwidget::sendSetTemperature(double temp) {
	double	t = temp + 273.15; // in kelvin
	if (_cooler) {
		try {
			_cooler->setTemperature(t);
		} catch (const std::exception& x) {
			coolerFailed(x);
			//displaySetTemperature(_cooler->getSetTemperature()
			//	- 273.15);
		}
	}
}

/**
 * \brief Turn the cooler on/off
 */
void	coolercontrollerwidget::activeToggled(bool active) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "turn the cooler %s",
		(active) ? "on" : "off");
	if (_cooler) {
		try {
			_cooler->setOn(active);
		} catch (const std::exception& x) {
			coolerFailed(x);
		}
	}
}

} // namespace snowgui
