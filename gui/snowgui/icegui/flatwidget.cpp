/*
 * flatwidget.cpp
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "flatwidget.h"
#include "ui_flatwidget.h"
#include <AstroDebug.h>
#include <IceConversions.h>
#include <imagedisplaywidget.h>
#include <ImageForwarder.h>

namespace snowgui {

/**
 * \brief Construct a new flat widget
 */
flatwidget::flatwidget(QWidget *parent)
	: calibrationimagewidget(parent), ui(new Ui::flatwidget) {
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
#if 0
	connect(this, SIGNAL(updateSignal(snowstar::CalibrationImageProgress)),
		this, SLOT(signalUpdated(snowstar::CalibrationImageProgress)));
	connect(this, SIGNAL(stopSignal()), this, SLOT(stopped()));
#endif
}

/**
 * \brief Destroy the flat widget
 */
flatwidget::~flatwidget() {
	delete ui;
}

/**
 * \brief Check for a flat image
 */
void	flatwidget::checkImage() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "checking for an image");
	try {
		snowstar::ImagePrx	image = _guider->flatImage();
		_image = snowstar::convert(image);
		emit offerImage(_image, imagetype());
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
void	flatwidget::statusUpdate() {
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "processing statusUpdate %d != %d",
		newstate, _guiderstate);
	try {
		ui->usedarkBox->setEnabled(_guider->hasDark());
	} catch (...) { }
	switch (newstate) {
	case snowstar::GuiderUNCONFIGURED:
	case snowstar::GuiderIDLE:
	case snowstar::GuiderCALIBRATED:
		ui->acquireButton->setEnabled(true);
		if (_image) {
			ui->viewButton->setEnabled(true);
		}
		ui->exposureBox->setEnabled(true);
		ui->numberBox->setEnabled(true);
		ui->usedarkBox->setEnabled(true);
		break;
	case snowstar::GuiderFLATACQUIRE:
		ui->exposureBox->setEnabled(false);
		ui->numberBox->setEnabled(false);
		ui->usedarkBox->setEnabled(false);
	case snowstar::GuiderCALIBRATING:
	case snowstar::GuiderGUIDING:
	case snowstar::GuiderDARKACQUIRE:
	case snowstar::GuiderIMAGING:
		ui->acquireButton->setEnabled(false);
		break;
	}
	_guiderstate = newstate;
	if (_acquiring && (newstate != snowstar::GuiderFLATACQUIRE)) {
		ui->propertyBox->setVisible(true);
		ui->progressWidget->setVisible(false);
		// retrieve the image
		checkImage();
	}
}

/**
 * \brief set the exposure time
 */
void	flatwidget::exposuretime(double e) {
	ui->exposureBox->setValue(e);
}

/**
 * \brief Acquire an image
 */
void	flatwidget::acquireClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "acquire clicked");
	try {
		double	exposuretime = ui->exposureBox->value();
		int	imagecount = ui->numberBox->value();
		bool	usedark = ui->usedarkBox->isChecked();
		_guider->startFlatAcquire(exposuretime, imagecount, usedark);
		_acquiring = true;
		snowstar::CalibrationImageProgress	prog;
		prog.imagecount = ui->numberBox->value();
		prog.imageno = 0;
		signalUpdated(prog);
		ui->propertyBox->setVisible(false);
		ui->progressWidget->setVisible(true);
		ui->exposureBox->setEnabled(false);
		ui->numberBox->setEnabled(false);
		ui->usedarkBox->setEnabled(false);
	} catch (const snowstar::BadState& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "bad state: %s", x.cause.c_str());
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", x.what());
	}
}

void	flatwidget::signalUpdated(snowstar::CalibrationImageProgress prog) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new signal received: imageno = %d",
		prog.imageno);
	ui->progressLabel->setText(QString(
		astro::stringprintf("Flat image progress: %d images of %d",
			prog.imageno, prog.imagecount).c_str()));
	ui->progressBar->setValue(100.0 * prog.imageno / prog.imagecount);
}

/**
 * \brief Stop the flat image process
 */
void    flatwidget::stopped() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stop");
}

} // namespace snowgui
