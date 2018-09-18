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

namespace snowgui {

/**
 * \brief Construct a calibration calculator widget
 */
calibrationcalculatordialog::calibrationcalculatordialog(
	snowstar::GuiderPrx guider,
	snowstar::GuiderFactoryPrx guiderfactory,
	snowstar::ControlType type, QWidget *parent)
	: QDialog(parent), _guider(guider), _guiderfactory(guiderfactory),
	  ui(new Ui::calibrationcalculatordialog) {
	ui->setupUi(this);

	// get information from the guider
	_focallength = _guider->getFocallength();
	ui->focallengthField->setText(QString(astro::stringprintf("%.3f", _focallength).c_str()));

	snowstar::CcdInfo	info = _guider->getCcd()->getInfo();
	_pixelsize = (info.pixelwidth + info.pixelheight) / 2;
	ui->pixelsizeField->setText(QString(astro::stringprintf("%.1f", _pixelsize * 1e6).c_str()));

	_angle = ui->angleSpinBox->value();

	_guiderate = _guider->getGuiderate();
	ui->guiderateField->setText(QString(astro::stringprintf("%.3f", _guiderate).c_str()));

	_declination = ui->declinationSpinBox->value();

	_rainvert = ui->rainvertCheckBox->isChecked();
	_decinvert = ui->decinvertCheckBox->isChecked();
	
	// initialize the calibration
	_cal.id = 0;
	_cal.timeago = 0;
	_cal.guider = _guider->getDescriptor();
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
	connect(ui->angleSpinBox, SIGNAL(valueChanged(int)),
		this, SLOT(angleChanged(int)));
	connect(ui->declinationSpinBox, SIGNAL(valueChanged(int)),
		this, SLOT(declinationChanged(int)));
	connect(ui->rainvertCheckBox, SIGNAL(stateChanged(int)),
		this, SLOT(rainvertChanged(int)));
	connect(ui->decinvertCheckBox, SIGNAL(stateChanged(int)),
		this, SLOT(decinvertChanged(int)));

	connect(ui->buttonBox, SIGNAL(accepted()),
		this, SLOT(acceptCalibration()));
	connect(ui->buttonBox, SIGNAL(rejected()),
		this, SLOT(rejectCalibration()));
	connect(this, SIGNAL(rejected()),
		this, SLOT(rejectCalibration()));

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
	_cal.coefficients[1] = -decsign * pixelspeed * sin(a);
	_cal.coefficients[4] =  decsign * pixelspeed * cos(a);
	int	rasign = (_rainvert) ? -1 : 1;
	pixelspeed = pixelspeed * cos(_declination * M_PI / 180);
	_cal.coefficients[0] =  rasign * pixelspeed * cos(a);
	_cal.coefficients[3] =  rasign * pixelspeed * sin(a);

	ui->calibrationDisplayWidget->setCalibration(_cal);
}

void	calibrationcalculatordialog::angleChanged(int angle) {
	_angle = angle;
	updateCalibration();
}

void	calibrationcalculatordialog::declinationChanged(int declination) {
	_declination = declination;
	updateCalibration();
}

void	calibrationcalculatordialog::rainvertChanged(int i) {
	_rainvert = (i) ? true : false;
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
	_guider->useCalibration(_cal.id, false);
	accept();
}

void	calibrationcalculatordialog::rejectCalibration() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration rejected");
	reject();
}

} // namespace snowgui
