/*
 * TrackingMonitorImage.cpp -- monitor to display the tracking images
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "TrackingMonitorImage.h"

namespace snowgui {

/**
 * \brief Create a tracking image monitor
 */
TrackingMonitorImage::TrackingMonitorImage(QObject *parent, QLabel *label)
	: MonitorImage(parent, label) {
}

/**
 * \brief destroy the image monitor
 */
TrackingMonitorImage::~TrackingMonitorImage() {
	unregister();
}

/**
 * \brief Setup the guider
 *
 * We also need an Ice::ObjectPtr that points to this object. We cannot
 * create this inside, because that would result in the impossibility to
 * to ever release this object again.
 */
void	TrackingMonitorImage::setGuider(snowstar::GuiderPrx guider,
		Ice::ObjectPtr myself) {
	if (_guider) {
		unregister();
	}
	_guider = guider;
	do_register(guider, myself);
	reregister();
}

/**
 * \brief Unregister from the guider
 */
void	TrackingMonitorImage::unregister() {
	if (_guider) {
		_guider->unregisterImageMonitor(_myidentity);
	}
}

/**
 * \brief Register with the guider
 */
void	TrackingMonitorImage::reregister() {
	if (!_guider) {
		return;
	}
	_guider->registerImageMonitor(_myidentity);
}

} // namespace snowgui
