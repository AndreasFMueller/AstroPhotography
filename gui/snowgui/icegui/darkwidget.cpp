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

namespace snowgui {

/**
 * \brief Construct a new dark widget
 */
darkwidget::darkwidget(QWidget *parent)
	: QDialog(parent), ui(new Ui::darkwidget) {
	ui->setupUi(this);


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
        statusTimer.setInterval(100);

	// application state
	_guiderstate = snowstar::GuiderUNCONFIGURED;
	_acquiring = false;
}

/**
 * \brief Destroy the dark widget
 */
darkwidget::~darkwidget() {
	statusTimer.stop();
	delete ui;
}

/**
 * \brief Check for a dark image
 */
void	darkwidget::checkImage() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "checking for an image");
	try {
		snowstar::ImagePrx	image = _guider->darkImage();
		_darkimage = snowstar::convert(image);
		image->remove();
		if (_darkimage) {
			ui->viewButton->setEnabled(true);
			ui->propertyTable->setImage(_darkimage);
		}
		emit newImage(_darkimage);
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"image acquire failed %s", x.what());
	}
}

/**
 * \brief set the guider
 */
void	darkwidget::guider(snowstar::GuiderPrx guider) {
	statusTimer.stop();
	_darkimage = astro::image::ImagePtr(NULL);
	_guider = guider;
	if (_guider) {
		checkImage();
		_guiderstate = snowstar::GuiderUNCONFIGURED;
		statusTimer.start();
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
		if (_darkimage) {
			ui->viewButton->setEnabled(true);
		}
		break;
	case snowstar::GuiderCALIBRATING:
	case snowstar::GuiderGUIDING:
	case snowstar::GuiderDARKACQUIRE:
	case snowstar::GuiderFLATACQUIRE:
	case snowstar::GuiderIMAGING:
		ui->acquireButton->setEnabled(false);
		ui->viewButton->setEnabled(false);
		break;
	}
	_guiderstate = newstate;
	if (_acquiring && (newstate != snowstar::GuiderDARKACQUIRE)) {
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
		_guider->startDarkAcquire(exposuretime, imagecount);
		_acquiring = true;
	} catch (const snowstar::BadState& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "bad state: %s", x.cause.c_str());
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", x.what());
	}
}

void	darkwidget::viewClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "view clicked");
	imagedisplaywidget	*id = new imagedisplaywidget(NULL);
	id->setImage(_darkimage);
	std::string	title = astro::stringprintf("dark image for %s",
		convert(_guider->getDescriptor()).toString().c_str());
	id->setWindowTitle(QString(title.c_str()));
	id->show();
}

/**
 * \brief handle window close event
 *
 * This event handler makes sure the window is destroyed when it is closed
 */
void    darkwidget::closeEvent(QCloseEvent * /* event */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "allow deletion");
	deleteLater();
}

} // namespace snowgui
