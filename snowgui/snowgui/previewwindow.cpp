/*
 * previewwindow.cpp -- implementation of the preview application
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "previewwindow.h"
#include "ui_previewwindow.h"
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <camera.h>
#include <QTimer>

using namespace astro::discover;

PreviewWindow::PreviewWindow(QWidget *parent, ServiceObject serviceobject,
	snowstar::RemoteInstrument instrument)
	: QWidget(parent),
	  _serviceobject(serviceobject), _instrument(instrument),
    	  ui(new Ui::PreviewWindow) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "starting PreviewWindow");
	ui->setupUi(this);

	// get the instrument name into the title
	std::string	title
		= astro::stringprintf("Preview instrument %s @ %s",
		_instrument.name().c_str(), _serviceobject.toString().c_str());
	setWindowTitle(QString(title.c_str()));

	debug(LOG_DEBUG, DEBUG_LOG, 0, "preview starting on instrument %s",
		_instrument.name().c_str());

	// read component names and initialize the combo boxes
	int	index = 0;
	while (_instrument.has(snowstar::InstrumentCCD, index)) {
		snowstar::CcdPrx	ccd = _instrument.ccd(index);
		if (!_ccd) {
			_ccd = ccd;
		}
		ui->ccdSelectionBox->addItem(QString(ccd->getName().c_str()));
		index++;
	}
	index = 0;
	while (_instrument.has(snowstar::InstrumentGuiderCCD, index)) {
		snowstar::CcdPrx	ccd = _instrument.guiderccd(index);
		if (!_ccd) {
			_ccd = ccd;
		}
		ui->ccdSelectionBox->addItem(QString(ccd->getName().c_str()));
		index++;
	}

	// cooler
	index = 0;
	while (_instrument.has(snowstar::InstrumentCooler, index)) {
		snowstar::CoolerPrx	cooler
			= _instrument.cooler(index);
		if (!_cooler) {
			_cooler = cooler;
		}
		ui->coolerSelectionBox->addItem(
			QString(cooler->getName().c_str()));
		index++;
	}

	// filterwheel
	index = 0;
	while (_instrument.has(snowstar::InstrumentFilterWheel, index)) {
		snowstar::FilterWheelPrx	filterwheel
			= _instrument.filterwheel(index);
		if (!_filterwheel) {
			_filterwheel = filterwheel;
		}
		ui->filterwheelSelectionBox->addItem(
			QString(filterwheel->getName().c_str()));
		index++;
	}

	// focuser
	index = 0;
	while (_instrument.has(snowstar::InstrumentFocuser, index)) {
		snowstar::FocuserPrx	focuser
			= _instrument.focuser(index);
		if (!_focuser) {
			_focuser = focuser;
		}
		ui->focuserSelectionBox->addItem(
			QString(focuser->getName().c_str()));
		index++;
	}

	// guiderport
	index = 0;
	while (_instrument.has(snowstar::InstrumentGuiderPort, index)) {
		snowstar::GuiderPortPrx	guiderport
			= _instrument.guiderport(index);
		if (!_guiderport) {
			_guiderport = guiderport;
		}
		ui->guiderportSelectionBox->addItem(
			QString(guiderport->getName().c_str()));
		index++;
	}

	// get information about the devices
	setupCcd();
	setupCooler();
	setupFilterwheel();
	setupFocuser();
	setupGuiderport();

	// start the timer
	statusTimer = new QTimer();
	connect(statusTimer, SIGNAL(timeout()), this, SLOT(statusUpdate()));
	statusTimer->setInterval(1000);
	statusTimer->start();
}

PreviewWindow::~PreviewWindow() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroy PreviewWindow");
	statusTimer->stop();
	delete statusTimer;
	delete ui;
}

void	PreviewWindow::setupCcd() {
	if (_ccd) {
		snowstar::CcdInfo	info = _ccd->getInfo();
		QComboBox	*bBox = ui->binningBox;
		std::for_each(info.binningmodes.begin(),
			info.binningmodes.end(),
			[bBox](const snowstar::BinningMode& mode) {
				std::string	m = astro::stringprintf("%dx%d",
					mode.x, mode.y);
				bBox->addItem(QString(m.c_str()));
			}
		);
	} else {
		while (ui->binningBox->count()) {
			ui->binningBox->removeItem(0);
		}
	}
}

void	PreviewWindow::setupFilterwheel() {
	if (_filterwheel) {
		ui->filternameBox->setEnabled(true);
		int	nfilterwheels = _filterwheel->nFilters();
		for (int i = 0; i < nfilterwheels; i++) {
			std::string	n = _filterwheel->filterName(i);
			std::string	item
				= astro::stringprintf("%d: %s", i+1, n.c_str());
			ui->filternameBox->addItem(QString(item.c_str()));
		}
		ui->filternameBox->setCurrentIndex(
			_filterwheel->currentPosition());
		if (snowstar::FwMOVING == _filterwheel->getState()) {
			ui->filterwheelStatus->setEnabled(true);
			ui->filterwheelStatus->setValue(0);
		} else {
			ui->filterwheelStatus->setEnabled(false);
			ui->filterwheelStatus->setValue(-1);
		}
	} else {
		ui->filternameBox->setEnabled(false);
		while (ui->filternameBox->count()) {
			ui->filternameBox->removeItem(0);
		}
	}
}

void	PreviewWindow::setupCooler() {
	if (_cooler) {
		float	t = _cooler->getActualTemperature() - 273.15;
		ui->actualTempField->setText(
			QString(astro::stringprintf("%.1f", t).c_str()));
		t = _cooler->getSetTemperature() - 273.15;
		ui->setTempSpinBox->setValue(t);
		ui->coolerOnButton->setEnabled(true);
		if (_cooler->isOn()) {
			ui->coolerOnButton->setText(QString("Off"));
		} else {
			ui->coolerOnButton->setText(QString("On"));
		}
	} else {
		ui->coolerOnButton->setText(QString("On"));
		ui->coolerOnButton->setEnabled(false);
	}
}

void	PreviewWindow::setupFocuser() {
	if (_focuser) {
		ui->focuserCurrentField->setEnabled(true);
		ui->focuserSet->setEnabled(true);
		ui->focuserSet->setSingleStep(10);
		ui->focuserSet->setMinimum(_focuser->min());
		ui->focuserSet->setMaximum(_focuser->max());
		int	s = _focuser->current();
		ui->focuserSet->setValue(s);
		ui->focuserCurrentField->setText(
			QString(astro::stringprintf("%d", s).c_str()));
	} else {
		ui->focuserCurrentField->setText(QString(""));
		ui->focuserCurrentField->setEnabled(false);
		ui->focuserSet->setEnabled(false);
	}
}

void	PreviewWindow::setupGuiderport() {
	if (_guiderport) {
		ui->raplusButton->setEnabled(true);
		ui->raminusButton->setEnabled(true);
		ui->decplusButton->setEnabled(true);
		ui->decminusButton->setEnabled(true);
	} else {
		ui->raplusButton->setEnabled(false);
		ui->raminusButton->setEnabled(false);
		ui->decplusButton->setEnabled(false);
		ui->decminusButton->setEnabled(false);
	}
}

void	PreviewWindow::statusUpdate() {
	if (_cooler) {
		float	t = _cooler->getActualTemperature() - 273.15;
		ui->actualTempField->setText(
			QString(astro::stringprintf("%.1f", t).c_str()));
		t = _cooler->getSetTemperature() - 273.15;
		ui->setTempSpinBox->setValue(t);
		ui->coolerOnButton->setEnabled(true);
	}
	if (_filterwheel) {
		if (snowstar::FwMOVING == _filterwheel->getState()) {
			ui->filterwheelStatus->setEnabled(true);
			ui->filterwheelStatus->setValue(0);
			ui->filterwheelStatus->setVisible(true);
		} else {
			ui->filterwheelStatus->setEnabled(false);
			ui->filterwheelStatus->setValue(-1);
			ui->filterwheelStatus->setVisible(false);
		}
	}
	if (_focuser) {
		int	s = _focuser->current();
		ui->focuserCurrentField->setText(
			QString(astro::stringprintf("%d", s).c_str()));
	}
	static const char	*white = "QButton { background-color : white; }";
	static const char	*transparent = "QButton { background-color : transparent; }";
	if (_guiderport) {
		unsigned char	a = _guiderport->active();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "activation: %01x", (int)a);
		if (a & snowstar::DECMINUS) {
			ui->decminusButton->setStyleSheet(white);
		} else {
			ui->decminusButton->setStyleSheet(transparent);
		}
		if (a & snowstar::DECPLUS) {
			ui->decplusButton->setStyleSheet(white);
		} else {
			ui->decplusButton->setStyleSheet(transparent);
		}
		if (a & snowstar::RAMINUS) {
			ui->raminusButton->setStyleSheet(white);
		} else {
			ui->raminusButton->setStyleSheet(transparent);
		}
		if (a & snowstar::RAPLUS) {
			ui->raplusButton->setStyleSheet(white);
		} else {
			ui->raplusButton->setStyleSheet(transparent);
		}
	}
}

void	PreviewWindow::ccdChanged(int ccdindex) {
	int	index = 0;
	while (_instrument.has(snowstar::InstrumentCCD, index)) {
		if (index == ccdindex) {
			_ccd = _instrument.ccd(index);
		}
		index++;
	}
	index = 0;
	while (_instrument.has(snowstar::InstrumentGuiderCCD, index)) {
		if (index == ccdindex) {
			_ccd = _instrument.ccd(index);
		}
		index++;
	}
	setupCcd();
}

void	PreviewWindow::filterwheelChanged(int filterwheelindex) {
	int	index = 0;
	while (_instrument.has(snowstar::InstrumentFilterWheel, index)) {
		if (filterwheelindex == index) {
			_filterwheel = _instrument.filterwheel(index);
		}
		index++;
	}
	setupFilterwheel();
}

void	PreviewWindow::filterwheelFilterChanged(int filterindex) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "select filter %d", filterindex);
	try {
		_filterwheel->select(filterindex);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot select: %s", x.what());
	}
}


void	PreviewWindow::coolerChanged(int coolerindex) {
	int	index = 0;
	while (_instrument.has(snowstar::InstrumentCooler, index)) {
		if (coolerindex == index) {
			_cooler = _instrument.cooler(index);
		}
		index++;
	}
	setupCooler();
}

void	PreviewWindow::coolerTemperatureChanged(double settemperature) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set temperature to %.1f",
		settemperature);
	_cooler->setTemperature(settemperature + 273.15);
}

void	PreviewWindow::coolerOnOff() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "toggle cooler");
	if (_cooler->isOn()) {
		_cooler->setOn(false);
		ui->coolerOnButton->setText(QString("On"));
	} else {
		_cooler->setOn(true);
		ui->coolerOnButton->setText(QString("Off"));
	}
}

void	PreviewWindow::focuserChanged(int focuserindex) {
	int	index = 0;
	while (_instrument.has(snowstar::InstrumentFocuser, index)) {
		if (focuserindex == index) {
			_focuser = _instrument.focuser(index);
		}
		index++;
	}
	setupFocuser();
}

void	PreviewWindow::focuserSetChanged(int focusposition) {
	_focuser->set(focusposition);
}

void	PreviewWindow::guiderportChanged(int guiderportindex) {
	int	index = 0;
	while (_instrument.has(snowstar::InstrumentGuiderPort, index)) {
		if (guiderportindex == index) {
			_guiderport = _instrument.guiderport(index);
		}
		index++;
	}
	setupGuiderport();
}

void	PreviewWindow::guiderportActivated() {
	static const float	defaultactivation = 5;
	if (sender() == ui->raplusButton) {
		_guiderport->activate(defaultactivation, 0);
	}
	if (sender() == ui->raminusButton) {
		_guiderport->activate(-defaultactivation, 0);
	}
	if (sender() == ui->decplusButton) {
		_guiderport->activate(0, defaultactivation);
	}
	if (sender() == ui->decminusButton) {
		_guiderport->activate(0, -defaultactivation);
	}
}

