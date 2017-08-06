/*
 * calibrationwidget.cpp -- calibration widget implementation
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "calibrationwidget.h"
#include "ui_calibrationwidget.h"
#include <calibrationselectiondialog.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <guidercontrollerwidget.h>
#include <calibrationdetaildialog.h>
#include <QTimer>

namespace snowgui {

/**
 * \brief Construct calibration widget
 */
calibrationwidget::calibrationwidget(QWidget *parent) :
	QWidget(parent), ui(new Ui::calibrationwidget) {
	ui->setupUi(this);
	connect(ui->databaseButton, SIGNAL(clicked()),
		this, SLOT(databaseClicked()));
	connect(ui->calibrateButton, SIGNAL(clicked()),
		this, SLOT(calibrateClicked()));
	connect(ui->detailButton, SIGNAL(clicked()),
		this, SLOT(detailClicked()));

	_state = snowstar::GuiderUNCONFIGURED;

	_guidercontroller = NULL;
	_calibration.id = -1;

	_statusTimer.setInterval(100);
	connect(&_statusTimer, SIGNAL(timeout()),
		this, SLOT(statusUpdate()));
}

/**
 * \brief Destroy the calibration widget
 */
calibrationwidget::~calibrationwidget() {
	_statusTimer.stop();
	delete ui;
}

/**
 * \brief set the guider information
 */
void	calibrationwidget::setGuider(snowstar::ControlType controltype,
		snowstar::GuiderDescriptor guiderdescriptor,
		snowstar::GuiderPrx guider,
		snowstar::GuiderFactoryPrx guiderfactory,
		guidercontrollerwidget *guidercontroller) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set up the guider %s|%d|%d|%d",
		_guiderdescriptor.instrumentname.c_str(),
		_guiderdescriptor.ccdIndex,
		_guiderdescriptor.guideportIndex,
		_guiderdescriptor.adaptiveopticsIndex);
	_controltype = controltype;
	_guiderdescriptor = guiderdescriptor;
	_guider = guider;
	_guiderfactory = guiderfactory;
	_guidercontroller = guidercontroller;

	// find out whether the guider is currently calibrated
	try {
		_calibration = _guider->getCalibration(_controltype);
		ui->calibrationdisplayWidget->setCalibration(_calibration);
		displayCalibration();
	} catch (...) {

	}

	// now that everything is configured, we start the timer
	_statusTimer.start();
}

/**
 * \brief Slot called when the databas button is clicked
 *
 * It opens a calibrationselectiondialog to select a clibration appropriate
 * for this device.
 */
void	calibrationwidget::databaseClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create a calibration selection");
	calibrationselectiondialog	*selection
		= new calibrationselectiondialog(this);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set up the guider in the selection");
	selection->setGuider(_controltype, _guiderdescriptor, _guiderfactory);
	connect(selection, SIGNAL(calibrationSelected(snowstar::Calibration)),
		this, SLOT(setCalibration(snowstar::Calibration)));
	selection->show();
}

/**
 * \brief Set the calibration
 */
void	calibrationwidget::setCalibration(snowstar::Calibration cal) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration %d selected", cal.id);
	_calibration = cal;
	ui->calibrationdisplayWidget->setCalibration(cal);
	displayCalibration();
	_guider->useCalibration(cal.id, false);
}

/**
* \brief display a calibration
 */
void	calibrationwidget::displayCalibration() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "display calibration %d",
		_calibration.id);
	if (_calibration.id <= 0) {
		ui->calibrationIDField->setText(QString(""));
		ui->numberField->setText(QString(""));
		ui->qualityField->setText(QString(""));
		ui->resolutionField->setText(QString(""));
		ui->intervalField->setText(QString(""));
		return;
	}
	ui->calibrationIDField->setText(QString::number(_calibration.id));
	ui->numberField->setText(QString::number(_calibration.points.size()));
	ui->qualityField->setText(QString(astro::stringprintf("%.1f%%",
		_calibration.quality * 100).c_str()));
	ui->intervalField->setText(QString(astro::stringprintf("%.1fs",
		_calibration.interval).c_str()));
	ui->resolutionField->setText(QString(astro::stringprintf("%.0f\"/px",
		_calibration.masPerPixel / 1000.).c_str()));
}

/**
 * \brief handle when calibrate was clicked
 */
void	calibrationwidget::calibrateClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start calibration for GuidePort");
	// make sure we have the most recent information on the
	// state
	setupState();
	if (_state == snowstar::GuiderCALIBRATING) {
		try {
			_guider->cancelCalibration();
		} catch (const std::exception& x) {
		}
	} else {
		if (_guidercontroller) {
			_guidercontroller->setupTracker();
		}
		try {
			_guider->startCalibration(_controltype);
		} catch (const std::exception& x) {
		}
	}
}

/**
 * \brief Display a calibration detail dialog
 */
void	calibrationwidget::detailClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "detail display requested");
	if (_calibration.id <= 0) {
		return;
	}
	calibrationdetaildialog	*cdd = new calibrationdetaildialog(this);
	cdd->setCalibration(_calibration);
	cdd->show();
}

/**
 * \brief Timer upate
 */
void	calibrationwidget::statusUpdate() {
	try {
		// find out whether something has changed in the state
		setupState();

		// check whether the calibration has changed
		snowstar::Calibration	cal = _guider->getCalibration(_controltype);
		if ((_calibration.id == cal.id)
			&& (_calibration.points.size() == cal.points.size())) {
			return;
		}
		_calibration = cal;
		ui->calibrationdisplayWidget->setCalibration(_calibration);
		displayCalibration();
	} catch (...) {

	}
}

/**
 * \brief Check whether the state has changed
 */
void	calibrationwidget::setupState() {
	snowstar::GuiderState	state = _guider->getState();
	if (state == _state) {
		return;
	}
	if (_state == snowstar::GuiderCALIBRATING) {
		try {
			_calibration = _guider->getCalibration(_controltype);
			ui->calibrationdisplayWidget->setCalibration(_calibration);
			displayCalibration();
		} catch (const std::exception& x) {
		}
	}
	_state = state;
	switch (_state) {
	case snowstar::GuiderUNCONFIGURED:
	case snowstar::GuiderIDLE:
	case snowstar::GuiderCALIBRATED:
		ui->calibrateButton->setText(QString("Calibrate"));
		ui->calibrateButton->setEnabled(true);
		ui->databaseButton->setEnabled(true);
		break;
	case snowstar::GuiderCALIBRATING:
		ui->calibrateButton->setText(QString("Stop"));
		ui->calibrateButton->setEnabled(true);
		ui->databaseButton->setEnabled(false);
		break;
	case snowstar::GuiderGUIDING:
		ui->calibrateButton->setText(QString("Calibrate"));
		ui->calibrateButton->setEnabled(false);
		ui->databaseButton->setEnabled(false);
		break;
	case snowstar::GuiderDARKACQUIRE:
	case snowstar::GuiderFLATACQUIRE:
	case snowstar::GuiderIMAGING:
		ui->calibrateButton->setEnabled(false);
		ui->databaseButton->setEnabled(false);
		break;
	}
}

} // namespace snowgui
