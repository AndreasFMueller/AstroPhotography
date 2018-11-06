/*
 * calibrationcalculatordialog.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller Hochschule Rapperswil
 */
#include "calibrationcalculatordialog.h"
#include "ui_calibrationcalculatordialog.h"
#include <cmath>
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <QTableWidgetItem>

namespace snowgui {

/**
 * \brief Construct a calibration calculator widget
 */
calibrationcalculatordialog::calibrationcalculatordialog(
	snowstar::GuiderPrx guider,
	snowstar::GuiderFactoryPrx guiderfactory,
	snowstar::ControlType type,
	calibrationwidget *calwidget,
	QWidget *parent)
	: QDialog(parent), _guider(guider), _guiderfactory(guiderfactory),
	  ui(new Ui::calibrationcalculatordialog) {
	ui->setupUi(this);

	ui->pixelsizeUnit->setText(QString("µm"));
	ui->angleUnit->setText(QString("º"));
	ui->declinationUnit->setText(QString("º"));

	// get information from the guider
	if (_guider) {
		_focallength = _guider->getFocallength();
		ui->focallengthField->setText(QString(
			astro::stringprintf("%.3f", _focallength).c_str()));

		snowstar::CcdInfo	info = _guider->getCcd()->getInfo();
		_pixelsize = (info.pixelwidth + info.pixelheight) / 2;
		ui->pixelsizeField->setText(QString(astro::stringprintf("%.1f",
			_pixelsize * 1e6).c_str()));

		_angle = ui->angleSpinBox->value();

		_guiderate = _guider->getGuiderate();
		ui->guiderateField->setText(QString(astro::stringprintf("%.3f",
			_guiderate).c_str()));
	} else {
		_focallength = 1;
		_pixelsize = 0.005;
		_angle = 0;
		_guiderate = 0.5;
	}

	_declination = ui->declinationSpinBox->value();

	_decinvert = ui->decinvertCheckBox->isChecked();
	
	// initialize the calibration
	_cal.id = 0;
	_cal.timeago = 0;
	if (_guider) {
		_cal.guider = _guider->getDescriptor();
	}
	_cal.coefficients = std::vector<float>(6);
	_cal.coefficients[0] = 1;
	_cal.coefficients[1] = 0;
	_cal.coefficients[2] = 0;
	_cal.coefficients[3] = 0;
	_cal.coefficients[4] = 1;
	_cal.coefficients[5] = 0;
	_cal.quality = 1;
	_cal.det = 1;
	_cal.complete = true;
	_cal.focallength = _focallength;
	_cal.masPerPixel = (_pixelsize / _focallength) * (180 * 3600 * 1000 / M_PI);;
	_cal.guiderate = _guiderate;
	_cal.interval = 0;
	_cal.type = type;
	_cal.flipped = false;

	// connect elements
	connect(ui->angleSpinBox, SIGNAL(valueChanged(double)),
		this, SLOT(angleChanged(double)));
	connect(ui->declinationSpinBox, SIGNAL(valueChanged(double)),
		this, SLOT(declinationChanged(double)));
	connect(ui->decinvertCheckBox, SIGNAL(stateChanged(int)),
		this, SLOT(decinvertChanged(int)));
	connect(ui->westCheckBox, SIGNAL(stateChanged(int)),
		this, SLOT(orientationChanged(int)));

	connect(ui->buttonBox, SIGNAL(accepted()),
		this, SLOT(acceptCalibration()));
	connect(ui->buttonBox, SIGNAL(rejected()),
		this, SLOT(rejectCalibration()));
	connect(this, SIGNAL(rejected()),
		this, SLOT(rejectCalibration()));

	connect(this, SIGNAL(newCalibration(snowstar::Calibration)),
		calwidget, SLOT(setCalibration(snowstar::Calibration)));

	debug(LOG_DEBUG, DEBUG_LOG, 0, "setting up coefficient table");
	for (int column = 0; column < 3; column++) {
		ui->coefficienttableWidget->setColumnWidth(column, 60);
		for (int row = 0; row < 2; row++) {
			QTableWidgetItem	*item = 
				new QTableWidgetItem(QString("0.00"));
			item->setTextAlignment(Qt::AlignRight);
			ui->coefficienttableWidget->setItem(row, column, item);
		}
	}
	ui->coefficienttableWidget->setRowHeight(0, 18);
	ui->coefficienttableWidget->setRowHeight(1, 18);

	// update the calibration
	updateCalibration();
}

/**
 * \brief Destroy the calibration calculator widget
 */
calibrationcalculatordialog::~calibrationcalculatordialog() {
	delete ui;
}

void	calibrationcalculatordialog::updateCalibration() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "recomputing calibration");
	double	speed = _guiderate * 2 * M_PI / 86400;
	double	pixelspeed = speed / _pixelsize;
	double	a = _angle * M_PI / 180;
	int	decsign = (_decinvert) ? -1 : 1;
	int	westsign = (_telescopewest) ? 1 : -1;
	_cal.coefficients[1] = -decsign * westsign * pixelspeed * sin(a);
	_cal.coefficients[4] =  decsign * westsign * pixelspeed * cos(a);
	pixelspeed = 2 * pixelspeed * cos(_declination * M_PI / 180);
	_cal.coefficients[0] = pixelspeed * westsign * cos(a);
	_cal.coefficients[3] = pixelspeed * westsign * sin(a);

	ui->coefficienttableWidget->item(0,0)->setText(
		QString(astro::stringprintf("%.2f",
			_cal.coefficients[0]).c_str()));
	ui->coefficienttableWidget->item(0,1)->setText(
		QString(astro::stringprintf("%.2f",
			_cal.coefficients[1]).c_str()));
	ui->coefficienttableWidget->item(0,2)->setText(
		QString(astro::stringprintf("%.2f",
			_cal.coefficients[2]).c_str()));
	ui->coefficienttableWidget->item(1,0)->setText(
		QString(astro::stringprintf("%.2f",
			_cal.coefficients[3]).c_str()));
	ui->coefficienttableWidget->item(1,1)->setText(
		QString(astro::stringprintf("%.2f",
			_cal.coefficients[4]).c_str()));
	ui->coefficienttableWidget->item(1,2)->setText(
		QString(astro::stringprintf("%.2f",
			_cal.coefficients[5]).c_str()));

	ui->calibrationDisplayWidget->setCalibration(_cal);
}

void	calibrationcalculatordialog::angleChanged(double angle) {
	_angle = angle;
	updateCalibration();
}

void	calibrationcalculatordialog::declinationChanged(double declination) {
	_declination = declination;
	updateCalibration();
}

void	calibrationcalculatordialog::decinvertChanged(int i) {
	_decinvert = (i) ? true : false;
	updateCalibration();
}

void	calibrationcalculatordialog::acceptCalibration() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration accepted");
	_cal.id = _guiderfactory->addCalibration(_cal);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration stored as %d", _cal.id);
	if (_guider) {
		_guider->useCalibration(_cal.id, false);
	}
	emit newCalibration(_cal);
	accept();
}

void	calibrationcalculatordialog::rejectCalibration() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration rejected");
	close();
}

void	calibrationcalculatordialog::setTelescope(astro::RaDec radec) {
	_declination = radec.dec().degrees();
	ui->declinationSpinBox->setValue(_declination);
	updateCalibration();
}

void	calibrationcalculatordialog::setOrientation(bool west) {
	_telescopewest = west;
	ui->westCheckBox->setChecked(_telescopewest);
	updateCalibration();
}

void	calibrationcalculatordialog::orientationChanged(int x) {
	setOrientation(x > 0);
}

} // namespace snowgui
