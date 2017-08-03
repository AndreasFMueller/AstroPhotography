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
	: QDialog(parent), ui(new Ui::flatwidget) {
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
 * \brief Destroy the flat widget
 */
flatwidget::~flatwidget() {
	statusTimer.stop();
	if (_imagedisplaywidget) {
		disconnect(_imagedisplaywidget, SLOT(destroyed()), 0, 0);
		_imagedisplaywidget->close();
	}
	delete ui;
}

/**
 * \brief Check for a flat image
 */
void	flatwidget::checkImage() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "checking for an image");
	try {
		snowstar::ImagePrx	image = _guider->flatImage();
		_flatimage = snowstar::convert(image);
		emit offerImage(_flatimage, std::string("flat"));
		image->remove();
		if (_flatimage) {
			ui->viewButton->setEnabled(true);
			ui->propertyTable->setImage(_flatimage);
		}
		emit newImage(_flatimage);
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"image acquire failed %s", x.what());
	}
}

/**
 * \brief set the guider
 */
void	flatwidget::guider(snowstar::GuiderPrx guider) {
	statusTimer.stop();
	_flatimage = astro::image::ImagePtr(NULL);
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
	switch (newstate) {
	case snowstar::GuiderUNCONFIGURED:
	case snowstar::GuiderIDLE:
	case snowstar::GuiderCALIBRATED:
		ui->acquireButton->setEnabled(true);
		if (_flatimage) {
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
	if (_acquiring && (newstate != snowstar::GuiderFLATACQUIRE)) {
		// retrieve the image
		checkImage();
	}
	try {
		ui->usedarkBox->setEnabled(_guider->hasDark());
	} catch (...) { }
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
	} catch (const snowstar::BadState& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "bad state: %s", x.cause.c_str());
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", x.what());
	}
}

void	flatwidget::viewClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "view clicked");
	if (_imagedisplaywidget) {
		_imagedisplaywidget->raise();
	} else {
		_imagedisplaywidget = new imagedisplaywidget(NULL);
		connect(_imagedisplaywidget, SIGNAL(destroyed()),
			this, SLOT(imageClosed()));
		connect(_imagedisplaywidget,
			SIGNAL(offerImage(astro::image::ImagePtr, std::string)),
			ImageForwarder::get(),
			SLOT(sendImage(astro::image::ImagePtr, std::string)));
		std::string	title = astro::stringprintf("flat image for %s",
			convert(_guider->getDescriptor()).name().c_str());
		_imagedisplaywidget->setWindowTitle(QString(title.c_str()));
		_imagedisplaywidget->show();
	}
	_imagedisplaywidget->setImage(_flatimage);
}

/**
 * \brief handle window close event
 *
 * This event handler makes sure the window is destroyed when it is closed
 */
void    flatwidget::closeEvent(QCloseEvent * /* event */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "allow deletion");
	emit closeWidget();
	deleteLater();
}

void	flatwidget::imageClosed() {
	_imagedisplaywidget = NULL;
}

void	flatwidget::changeEvent(QEvent *event) {
	if (this->window()->isActiveWindow()) {
		emit offerImage(_flatimage, std::string("flat"));
	}
	QWidget::changeEvent(event);
}

} // namespace snowgui
