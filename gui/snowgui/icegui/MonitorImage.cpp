/*
 * MonitorImage.cpp
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "MonitorImage.h"
#include <AstroDebug.h>
#include <CommunicatorSingleton.h>
#include <CommonClientTasks.h>
#include <QImage>

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
	_image.size.width = 0;
	_image.size.height = 0;
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
	} else if (s < 0) {
		_scale = 0;
	} else {
		_scale = s;
	}
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
void	MonitorImage::update(const snowstar::SimpleImage& image,
		const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new %dx%d image received",
		image.size.width, image.size.height);
	if (_freeze) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "monitor frozen, image lost");
		return;
	}
	_image = image;
	rebuildImage();

	// emit the signal
	emit imageUpdated();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "signal emitted");
}

/**
 * \brief Convert to a displayable image
 */
void	MonitorImage::rebuildImage() {
	if ((0 == _image.size.width) || (0 == _image.size.height)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no image, giving up");
	}
	// first find maximum and minimum value
	unsigned short	min = std::numeric_limits<unsigned short>::max();
	unsigned short	max = std::numeric_limits<unsigned short>::min();
	std::for_each(_image.imagedata.begin(), _image.imagedata.end(),
		[&min,&max](unsigned short pixel) {
			if (pixel < min) {
				min = pixel;
			}
			if (pixel > max) {
				max = pixel;
			}
		}
	);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "minimum = %hu, maximum = %hu",
		min, max);
	struct conversion_parameters	cp;
	cp.scale = 255. / (max - min);
	if (_inverse) {
		cp.offset = max;
		cp.scale = -cp.scale;
		cp.limit = 1 << _scale;
	} else {
		cp.offset = min;
		cp.limit = 1 << _scale;
	}

	// convert the SimpleImage received into a QImage
	cp.width = _image.size.width;
	cp.height = _image.size.height;
	QImage  *qimage = new QImage(cp.limit * cp.width, cp.limit * cp.height,
		QImage::Format_RGB32);
	int	counter = 0;
	std::for_each(_image.imagedata.begin(), _image.imagedata.end(),
		[qimage,cp,&counter](unsigned short pixel) mutable {
			int	x = cp.limit * (counter % cp.width);
			int	y = cp.limit * (cp.height - 1 - counter / cp.width);
			unsigned char	v = (pixel - cp.offset) * cp.scale;
			unsigned long	value = 0xff000000 | (v << 16) | (v << 8) | v;
			for (int xi = 0; xi < cp.limit; xi++)  {
				for (int yi = 0; yi < cp.limit; yi++) {
					qimage->setPixel(x + xi, y + yi, value);
				}
			}
			counter++;
		}
	);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d pixels processed", counter);

	// convert the QImage to a pixmap
	QPixmap *result = new QPixmap(cp.limit * cp.width, cp.limit * cp.height);
	result->convertFromImage(*qimage);
	if (_pixmap) {
		delete _pixmap;
	}
	_pixmap = result;
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
