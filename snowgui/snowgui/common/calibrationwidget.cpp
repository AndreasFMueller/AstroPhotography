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

	_guidercontroller = NULL;
}

/**
 * \brief Destroy the calibration widget
 */
calibrationwidget::~calibrationwidget() {
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
		_guiderdescriptor.guiderportIndex,
		_guiderdescriptor.adaptiveopticsIndex);
	_controltype = controltype;
	_guiderdescriptor = guiderdescriptor;
	_guider = guider;
	_guiderfactory = guiderfactory;
	_guidercontroller = guidercontroller;

	// find out whether the guider is currently calibrated
	try {
		_calibration = _guider->getCalibration(controltype);
		ui->calibrationdisplayWidget->setCalibration(_calibration);
		displayCalibration();
	} catch (...) {

	}
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
	selection->exec();
}

/**
 * \brief Set the calibration
 */
void	calibrationwidget::setCalibration(snowstar::Calibration cal) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration %d selected", cal.id);
	ui->calibrationdisplayWidget->setCalibration(cal);
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
		return;
	}
	ui->calibrationIDField->setText(QString::number(_calibration.id));
	ui->numberField->setText(QString::number(_calibration.points.size()));
	ui->qualityField->setText(QString(astro::stringprintf("%.1f%%",
		_calibration.quality * 100).c_str()));
	ui->resolutionField->setText(QString(astro::stringprintf("%.0fmas/px",
		_calibration.masPerPixel).c_str()));
}

/**
 * \brief handle when calibrate was clicked
 */
void	calibrationwidget::calibrateClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start calibration for GuiderPort");
	if (_guidercontroller) {
		_guidercontroller->setupTracker();
	}
	_guider->startCalibration(_controltype);
}

} // namespace snowgui
