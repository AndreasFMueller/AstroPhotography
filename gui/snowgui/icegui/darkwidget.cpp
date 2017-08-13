/*
 * darkwidget.cpp
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "darkwidget.h"
#include "ui_darkwidget.h"
#include <AstroDebug.h>
#include <IceConversions.h>
#include <imagedisplaywidget.h>
#include <ImageForwarder.h>

namespace snowgui {

/**
 * \brief Construct a new dark widget
 */
darkwidget::darkwidget(QWidget *parent)
	: calibrationimagewidget(parent), ui(new Ui::darkwidget) {
	ui->setupUi(this);
	ui->progressWidget->setVisible(false);

	// make the buttons disabled
	ui->acquireButton->setAutoDefault(false);
	ui->acquireButton->setEnabled(false);
	ui->viewButton->setAutoDefault(false);
	ui->viewButton->setEnabled(false);

	// connections
	connect(ui->acquireButton, SIGNAL(clicked()),
		this, SLOT(acquireClicked()));
	connect(ui->viewButton, SIGNAL(clicked()),
		this, SLOT(viewClicked()));

	// program the timer
	connect(&statusTimer, SIGNAL(timeout()), this, SLOT(statusUpdate()));

	// connect the progress info singal
	connect(this, SIGNAL(updateSignal(snowstar::CalibrationImageProgress)),
		this, SLOT(signalUpdated(snowstar::CalibrationImageProgress)));
	connect(this, SIGNAL(stopSignal()), this, SLOT(stopped()));
}

/**
 * \brief Destroy the dark widget
 */
darkwidget::~darkwidget() {
	delete ui;
}

/**
 * \brief Check for a dark image
 */
void	darkwidget::checkImage() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "checking for an image");
	try {
		snowstar::ImagePrx	image = _guider->darkImage();
		_image = snowstar::convert(image);
		emit offerImage(_image, std::string("dark"));
		image->remove();
		_acquiring = false;
		if (_image) {
			ui->viewButton->setEnabled(true);
			ui->propertyTable->setImage(_image);
		} else {
			ui->viewButton->setEnabled(false);
		}
		emit newImage(_image);
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"image acquire failed %s", x.what());
	}
}

/**
 * \brief Slot called when the timer detects a status update
 */
void	darkwidget::statusUpdate() {
	if (!_guider) {
		return;
	}
	snowstar::GuiderState	newstate;
	try {
		newstate = _guider->getState();
	} catch (...) {
		return;
	}
	if (_guiderstate == newstate) {
		return;
	}
	switch (newstate) {
	case snowstar::GuiderUNCONFIGURED:
	case snowstar::GuiderIDLE:
	case snowstar::GuiderCALIBRATED:
		ui->acquireButton->setEnabled(true);
		if (_image) {
			ui->viewButton->setEnabled(true);
		}
		ui->exposureBox->setEnabled(true);
		ui->hotlimitBox->setEnabled(true);
		ui->numberBox->setEnabled(true);
		break;
	case snowstar::GuiderDARKACQUIRE:
		ui->exposureBox->setEnabled(false);
		ui->hotlimitBox->setEnabled(false);
		ui->numberBox->setEnabled(false);
	case snowstar::GuiderCALIBRATING:
	case snowstar::GuiderGUIDING:
	case snowstar::GuiderFLATACQUIRE:
	case snowstar::GuiderIMAGING:
	case snowstar::GuiderBACKLASH:
		ui->acquireButton->setEnabled(false);
		break;
	}
	_guiderstate = newstate;
	if (_acquiring && (newstate != snowstar::GuiderDARKACQUIRE)) {
		ui->propertyBox->setVisible(true);
		ui->progressWidget->setVisible(false);
		// retrieve the image
		checkImage();
	}
}

/**
 * \brief set the exposure time
 */
void	darkwidget::exposuretime(double e) {
	ui->exposureBox->setValue(e);
}

/**
 * \brief Acquire an image
 */
void	darkwidget::acquireClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "acquire clicked");
	try {
		double	exposuretime = ui->exposureBox->value();
		int	imagecount = ui->numberBox->value();
		double	badpixellimit = ui->hotlimitBox->value();
		_guider->startDarkAcquire(exposuretime, imagecount,
			badpixellimit);
		_acquiring = true;
		snowstar::CalibrationImageProgress	prog;
		prog.imagecount = ui->numberBox->value();
		prog.imageno = 0;
		signalUpdated(prog);
		ui->propertyBox->setVisible(false);
		ui->progressWidget->setVisible(true);
		ui->exposureBox->setEnabled(false);
		ui->hotlimitBox->setEnabled(false);
		ui->numberBox->setEnabled(false);
	} catch (const snowstar::BadState& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "bad state: %s", x.cause.c_str());
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", x.what());
	}
}

/**
 * \brief Update the progress indicator
 */
void	darkwidget::signalUpdated(snowstar::CalibrationImageProgress prog) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new signal received: imageno = %d",
		prog.imageno);
	ui->progressLabel->setText(QString(
		astro::stringprintf("Dark image progress: %d images of %d",
			prog.imageno, prog.imagecount).c_str()));
	ui->progressBar->setValue(100.0 * prog.imageno / prog.imagecount);
}

/**
 * \brief Stop the dark image process
 */
void	darkwidget::stopped() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stop");
}

} // namespace snowgui
