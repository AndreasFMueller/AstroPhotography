/*
 * calibrationimagewidget.cpp -- implementation of the calibration image widget
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "calibrationimagewidget.h"
#include <AstroDebug.h>
#include <CommunicatorSingleton.h>
#include <ImageForwarder.h>
#include <IceConversions.h>

namespace snowgui {

/**
 * \brief Create a new calibrationimagewidget
 */
calibrationimagewidget::calibrationimagewidget(QWidget *parent)
	: QDialog(parent) {

	_imagedisplaywidget = NULL;
	statusTimer.setInterval(100);
	_guiderstate = snowstar::GuiderUNCONFIGURED;
	_acquiring = false;

	qRegisterMetaType<snowstar::CalibrationImageProgress>("snowstar::CalibrationImageProgress");
}

/**
 * \brief Destroy the widget
 */
calibrationimagewidget::~calibrationimagewidget() {
	statusTimer.stop();
	if (_imagedisplaywidget) {
		disconnect(_imagedisplaywidget, SIGNAL(destroyed()), 0, 0);
	}
}

/**
 * \brief install the guider
 *
 * This method also registers the widget as a monitor for calibration image
 * updates.
 */
void	calibrationimagewidget::guider(snowstar::GuiderPrx guider) {
	try {
		do_unregister();
	} catch (...) { }
	_monitor = new CalibrationImageMonitor(this);
	_image = astro::image::ImagePtr(NULL);
	_guider = guider;
	if (_guider) {
		_guiderstate = snowstar::GuiderUNCONFIGURED;
		checkImage();
		statusTimer.start();
	}
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"registering the calibrationimagewidget");
		snowstar::CommunicatorSingleton::connect(guider);
		Ice::ObjectPtr	_monitorptr(_monitor);
		_monitoridentity = snowstar::CommunicatorSingleton::add(_monitorptr);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "identity %s",
			_monitoridentity.name.c_str());
		_guider->registerCalibrationImageMonitor(_monitoridentity);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot register %s", x.what());
	}
	connect(_monitor, SIGNAL(stopSignal()),
		this, SLOT(stopped()));
	connect(_monitor,
		SIGNAL(updateSignal(snowstar::CalibrationImageProgress)),
		this,
		SLOT(signalUpdated(snowstar::CalibrationImageProgress)));
}

/**
 * \brief Unregister the widget as a servant
 */
void	calibrationimagewidget::do_unregister() {
	disconnect(_monitor, SIGNAL(stopSignal()), 0, 0);
	disconnect(_monitor,
		SIGNAL(updateSignal(snowstar::CalibrationImageProgress)),
		0, 0);
	if (_guider) {
		_guider->unregisterCalibrationImageMonitor(_monitoridentity);
	}
	snowstar::CommunicatorSingleton::remove(_monitoridentity);
	_monitor = NULL;
}

/**
 * \brief Common slot opening the calibration image in a new window
 */
void	calibrationimagewidget::viewClicked() {
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
		std::string     title
			= astro::stringprintf("%s image for %s",
				imagetype().c_str(),
				convert(_guider->getDescriptor()).name()
					.c_str());
		_imagedisplaywidget->setWindowTitle(QString(title.c_str()));
		_imagedisplaywidget->show();
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "loading %s image", imagetype().c_str());
	if (_image) {
		_imagedisplaywidget->setImage(_image);
	}
}

/**
 * \brief Processing the close event
 *
 * This involves unregistering the widget as a servant
 */
void	calibrationimagewidget::closeEvent(QCloseEvent * /* event */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "allow deletion");
	do_unregister();
	emit closeWidget();
	emit offerImage(ImagePtr(NULL), std::string());
	deleteLater();
}

/**
 * \brief 
 */
void	calibrationimagewidget::changeEvent(QEvent *event) {
	if (this->window()->isActiveWindow()) {
		emit offerImage(_image, imagetype());
	}
	QWidget::changeEvent(event);
}

void	calibrationimagewidget::imageClosed() {
	_imagedisplaywidget = NULL;
}

} // namespace snowgui
