/*
 * MonitorImage.cpp
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "MonitorImage.h"
#include <AstroDebug.h>
#include <CommunicatorSingleton.h>
#include <CommonClientTasks.h>
#include <IceConversions.h>
#include <QImage>
#include <Image2Pixmap.h>
#include <AstroFilterfunc.h>

namespace snowgui {

/**
 * \brief Construct a MonitorImage
 */
MonitorImage::MonitorImage(QObject *parent, QLabel *label)
	: QObject(parent), _label(label) {
	_pixmap = NULL;
	connect(this, SIGNAL(imageUpdated()), this, SLOT(refreshImage()),
		Qt::QueuedConnection);
	_scale = 0;
	_freeze = false;
	_inverse = false;
}

/**
 * \brief Destroy the MonitorImage
 *
 * The destructor has not much to do. But in a derived class, the destructor
 * should take care of unregistering from the server.
 */
MonitorImage::~MonitorImage() {
	if (_pixmap) {
		delete _pixmap;
	}
	disconnect(this, SIGNAL(imageUpdated()), this, SIGNAL(refreshImage()));
}

/**
 * \brief Set the scale factor
 */
void	MonitorImage::setScale(int s) {
	if (s > 3) {
		_scale = 3;
	} else if (s < -3) {
		_scale = -3;
	} else {
		_scale = s;
	}
	rebuildImage();
	refreshImage();
}

/**
 * \brief Set the inverse flag
 */
void	MonitorImage::setInverse(bool i) {
	_inverse = i;
	rebuildImage();
	refreshImage();
}

/**
 * \brief freeze updates
 */
void	MonitorImage::setFreeze(bool f) {
	_freeze = f;
}

/**
 * \brief Stop callback method
 *
 * The server sends this method when a calibration or tracking has stopped.
 * In our case we can simply ignore it. Override it in a derived class if
 * you want a reaction to the stop signal.
 */
void	MonitorImage::stop(const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got stop signal");
	emit streamStopped();
}

struct conversion_parameters {
	int	width;
	int	height;
	double	scale;
	int	offset;
	int	_scale;
	int	limit;
};

/**
 * \brief Update callback function
 *
 * This method is called by the callback, so it may not be executed on Qt's
 * main thread. This means that it can only do things that are allowed to be
 * done on the main thread. It then emits the imageUpdated() signal, and the
 * refreshImage() slot will make sure the new image is displayd
 */
void	MonitorImage::update(const snowstar::ImageBuffer& image,
		const Ice::Current& /* current */) {
	if (_freeze) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "monitor frozen, image lost");
		return;
	}

	// convert the image from the buffer into an ImagePtr
	_image = convertimage(image);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new %s image received",
		_image->size().toString().c_str());
	rebuildImage();

	// emit the signal
	emit imageUpdated();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "signal emitted");
}

/**
 * \brief Convert to a displayable image
 */
void	MonitorImage::rebuildImage() {
	if (!_image) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no image, giving up");
		return;
	}

	std::unique_lock<std::recursive_mutex>	lock(_mutex);

	if (_pixmap) {
		delete _pixmap;
		_pixmap = NULL;
	}
	Image2Pixmap	i2p;
	i2p.scale(_scale);
	i2p.negative(_inverse);
	if (_image->bytesPerPlane() > 1) {
		// find the maximum and minimum values and compute the
		// gain and brightness from this
		double	max = astro::image::filter::max_luminance(_image);
		double	min = astro::image::filter::min_luminance(_image);
		double	gain = 256 / (max - min);
		i2p.gain(gain);
		double	brightness = -min * gain;
		i2p.brightness(brightness);
	}
	_pixmap = i2p(_image);
}

/**
 * \brief Redisplay the internally computed image
 *
 * This is the second half of the processing of incoming images. The first
 * half is what is done in the update method. In the second half, the image
 * is actually displayed in the widget.
 */
void	MonitorImage::refreshImage() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "refresh image slot called");
	// take pixmap and display it in the label
	if ((_label) && (_pixmap)) {
		_label->setPixmap(*_pixmap);
		_label->setFixedSize(_pixmap->width(), _pixmap->height());
		_label->setMinimumSize(_pixmap->width(), _pixmap->height());
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"cannot set pixmap (%p) in label (%p)",
			_pixmap, _label);
	}
}

/**
 * \brief Register with the adapter and the proxy
 *
 * The myself argument points to an image 
 */
void	MonitorImage::do_register(Ice::ObjectPrx proxy, Ice::ObjectPtr myself) {
        snowstar::CommunicatorSingleton::connect(proxy);
        _myidentity = snowstar::CommunicatorSingleton::add(myself);
	snowstar::CommunicatorSingleton::connect(proxy);
}

void	MonitorImage::do_unregister() {
        snowstar::CommunicatorSingleton::remove(_myidentity);
}

} // namespace snowgui
