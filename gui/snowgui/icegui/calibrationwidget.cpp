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
#include <calibrationcalculatordialog.h>
#include <QTimer>
#include <IceConversions.h>

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
	connect(ui->calculateButton, SIGNAL(clicked()),
		this, SLOT(calculateClicked()));
	connect(ui->detailButton, SIGNAL(clicked()),
		this, SLOT(detailClicked()));

	_state = snowstar::GuiderUNCONFIGURED;

	_guidercontroller = NULL;
	_calibration.id = -1;

	_statusTimer.setInterval(100);
	connect(&_statusTimer, SIGNAL(timeout()),
		this, SLOT(statusUpdate()));

	ui->calibrationIDField->setText(QString(""));
	ui->numberField->setText(QString(""));
	ui->positionField->setText(QString(""));
	ui->resolutionField->setText(QString(""));
	ui->qualityField->setText(QString(""));
	ui->intervalField->setText(QString(""));
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
		const std::string& instrumentname,
		snowstar::GuiderPrx guider,
		snowstar::GuiderFactoryPrx guiderfactory,
		guidercontrollerwidget *guidercontroller) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set up the guider %s",
		instrumentname.c_str());
	_controltype = controltype;
	_instrumentname = instrumentname;
	_guider = guider;
	_guiderfactory = guiderfactory;
	_guidercontroller = guidercontroller;

	if ((!_guiderfactory) || (!_guider)) {
		debug(LOG_ERR, DEBUG_LOG, 0, "no guider (factory)");
		return;
	}

	// now that everything is configured, we start the timer
	_statusTimer.start();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "statusTimer started");

	// find out whether the guider is currently calibrated
	try {
		_calibration = _guider->getCalibration(_controltype);
		ui->calibrationdisplayWidget->setCalibration(_calibration);
		displayCalibration();
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot get calibration: %s",
			x.what());
		return;
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
	selection->setGuider(_controltype, _instrumentname, _guiderfactory);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "guider set");
	connect(selection, SIGNAL(calibrationSelected(snowstar::Calibration)),
		this, SLOT(setCalibration(snowstar::Calibration)));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "showing selection");
	selection->show();
}

/**
 * \brief Set the calibration
 */
void	calibrationwidget::setCalibration(snowstar::Calibration cal) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration %d selected, position %s",
		cal.id, (cal.east) ? "east" : "west");
	_calibration = cal;
	ui->calibrationdisplayWidget->setCalibration(_calibration);
	displayCalibration();
	if (_guider) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "set cal %d in guider",
			_calibration.id);
		_guider->useCalibration(_calibration.id, false);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "emit calibrationChanged(), cal = %d",
		_calibration.id);
	emit calibrationChanged();
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
	std::string	positionlabel = astro::stringprintf("%s/𝛿=%.1fº",
		(_calibration.east) ? "east" : "west",
		_calibration.declination);
;	ui->positionField->setText(QString(positionlabel.c_str()));
	ui->qualityField->setText(QString(astro::stringprintf("%.1f%%",
		_calibration.quality * 100).c_str()));
	ui->resolutionField->setText(QString(astro::stringprintf("%.0f\"/px",
		_calibration.masPerPixel / 1000.).c_str()));

	// compute the number of pixels offset we expect from the interval
	double	speed = _calibration.guiderate * (360. * 3600. / 86400.); // as/s
	double	offset = _calibration.interval * speed;	// arcsec
	int	pixeloffset = offset / (_calibration.masPerPixel / 1000.);
	ui->intervalField->setText(QString(astro::stringprintf("%.1fs/%dpx",
		_calibration.interval, pixeloffset).c_str()));
}

/**
 * \brief handle when calibrate was clicked
 */
void	calibrationwidget::calibrateClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start calibration for GuidePort");
	if (!_guider) {
		debug(LOG_ERR, DEBUG_LOG, 0, "no guider present");
		return;
	}
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
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"current declination=%.f",
				_radec.dec().degrees());
			// XXX we should get the gridpixels from the
			// XXX gui, value 0 means ignore it
			_guider->startCalibration(_controltype, 0., !_west,
				_radec.dec().degrees());
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
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "status update");
	try {
		// find out whether something has changed in the state
		setupState();

		// check whether the calibration has changed
		snowstar::Calibration	cal
			= _guider->getCalibration(_controltype);
		if ((_calibration.id == cal.id)
			&& (_calibration.points.size() == cal.points.size())) {
			return;
		}
		_calibration = cal;
		ui->calibrationdisplayWidget->setCalibration(_calibration);
		displayCalibration();
		emit calibrationChanged();
	} catch (...) {

	}
}

/**
 * \brief Check whether the state has changed
 */
void	calibrationwidget::setupState() {
	if (!_guider) {
		return;
	}
	snowstar::GuiderState	state = snowstar::GuiderUNCONFIGURED;
	try {
		state = _guider->getState();
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot get guider state: %s",
			x.what());
	}
	if (state == _state) {
		return;
	}
	if (_state == snowstar::GuiderCALIBRATING) {
		try {
			_calibration = _guider->getCalibration(_controltype);
			ui->calibrationdisplayWidget->setCalibration(_calibration);
			displayCalibration();
			emit calibrationChanged();
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
	case snowstar::GuiderBACKLASH:
		ui->calibrateButton->setEnabled(false);
		ui->databaseButton->setEnabled(false);
		break;
	}
}

/**
 * \brief Display the calculator dialog
 */
void	calibrationwidget::calculateClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calculate clicked");
	calibrationcalculatordialog	*ccd
		= new calibrationcalculatordialog(_guider, _guiderfactory,
			_controltype, this, this);
	// get the data that we alread have
	ccd->setTelescope(_radec);
	connect(this, SIGNAL(telescopeChanged(astro::RaDec)),
		ccd, SLOT(setTelescope(astro::RaDec)));

	ccd->setOrientation(_west);
	connect(this, SIGNAL(orientationChanged(bool)),
		ccd, SLOT(setOrientation(bool)));

	ccd->exec();
}

void    calibrationwidget::setTelescope(astro::RaDec radec) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new telescope: %s",
		radec.toString().c_str());
	_radec = radec;
	emit telescopeChanged(_radec);
}

void	calibrationwidget::setOrientation(bool west) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new orientation: %s",
		(west) ? "west" : "east");
	_west = west;
	emit orientationChanged(_west);
}

} // namespace snowgui
