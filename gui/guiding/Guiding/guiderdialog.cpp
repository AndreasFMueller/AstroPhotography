#include "guiderdialog.h"
#include "ui_guiderdialog.h"
#include <image.hh>
#include <AstroDebug.h>
#include <QWidget>
#include <guidermonitordialog.h>

GuiderDialog::GuiderDialog(Astro::Guider_var guider, QWidget *parent) :
	QDialog(parent), _guider(guider), ui(new Ui::GuiderDialog)
{
    ui->setupUi(this);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "guider dialog created");

        Astro::GuiderDescriptor_var     descriptor = _guider->getDescriptor();

	// write window title
        char    buffer[1024];
        snprintf(buffer, sizeof(buffer), "%s|%d|%s",
		&*(descriptor->cameraname),
                descriptor->ccdid,
		&*(descriptor->guiderportname));
        debug(LOG_DEBUG, DEBUG_LOG, 0, "title: %s", buffer);
        this->setWindowTitle(buffer);

	// query the ccd for the supported binning modes, and display
	// the valid binning modes 
	ui->binningcomboBox->clear();
	Astro::Ccd_var	ccd = _guider->getCcd();
	Astro::CcdInfo_var	info = ccd->getInfo();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "maximum size: %dx%d",
		info->size.width, info->size.height);
	for (unsigned int i = 0; i < info->binningmodes.length(); i++) {
		char	buffer[20];
		snprintf(buffer, sizeof(buffer), "%dx%d",
			info->binningmodes[i].x, info->binningmodes[i].y);
		ui->binningcomboBox->addItem(buffer);
	}
	ui->binningcomboBox->setCurrentIndex(info->binningmodes.length() - 1);

	// exposure initialization
	Astro::Exposure	exposure = _guider->getExposure();
	setExposure(exposure);

	// set guider state
	setGuiderState(_guider->getState());
	setStar(_guider->getStar());
}

GuiderDialog::~GuiderDialog()
{
	delete ui;
}

void	GuiderDialog::setExposure(const Astro::Exposure& exposure) {
	ui->timeSpinbox->setValue(exposure.exposuretime);
	char	buffer[21];
	snprintf(buffer, sizeof(buffer), "%dx%d",
		exposure.mode.x, exposure.mode.y);
	for (int i = 0; i < ui->binningcomboBox->count(); i++) {
		if (ui->binningcomboBox->itemText(i) == QString(buffer)) {
			ui->binningcomboBox->setCurrentIndex(i);
		}
	}
}

/**
 * \brief Event handler for exposure time changes
 */
void	GuiderDialog::exposuretime(double t) {
	Astro::Exposure	exposure = _guider->getExposure();
	exposure.exposuretime = t;
	_guider->setExposure(exposure);
}

/**
 * \brief update guider state
 */
void	GuiderDialog::setGuiderState(const Astro::Guider::GuiderState& guiderstate) {
	ui->captureButton->setText("Capture");
	ui->calibrateButton->setText("Calibrate");
	ui->guideButton->setText("Guide");
	switch (guiderstate) {
	case Astro::Guider::GUIDER_UNCONFIGURED:
		ui->captureButton->setEnabled(true);
		ui->calibrateButton->setEnabled(false);
		ui->calibrateButton->setText("Calibrate: unconfigured");
		ui->guideButton->setEnabled(false);
		ui->guideButton->setText("Guide: unconfigured");
		break;
	case Astro::Guider::GUIDER_IDLE:
		ui->captureButton->setEnabled(true);
		ui->calibrateButton->setEnabled(true);
		ui->guideButton->setEnabled(false);
		ui->guideButton->setText("Guider: uncalibrated");
		break;
	case Astro::Guider::GUIDER_CALIBRATING:
		ui->captureButton->setEnabled(false);
		ui->captureButton->setText("Capture: calibrating");
		ui->calibrateButton->setEnabled(true);
		ui->calibrateButton->setText("Cancel calibration");
		ui->guideButton->setEnabled(false);
		ui->guideButton->setText("Guide: calibrating");
		break;
	case Astro::Guider::GUIDER_CALIBRATED:
		ui->captureButton->setEnabled(true);
		ui->calibrateButton->setEnabled(true);
		ui->guideButton->setEnabled(true);
		break;
	case Astro::Guider::GUIDER_GUIDING:
		ui->captureButton->setEnabled(false);
		ui->captureButton->setText("Capture: guiding");
		ui->calibrateButton->setEnabled(false);
		ui->calibrateButton->setText("Calibrate: guiding");
		ui->guideButton->setEnabled(true);
		ui->guideButton->setText("Cancel guiding");
		break;
	}
}

void	GuiderDialog::setStar(const Astro::Point& star) {
	ui->starxField->setText(tr("%1").arg(star.x));
	ui->staryField->setText(tr("%1").arg(star.y));
}

/**
 * \brief capture a full size image from the CCD
 *
 * During the capture, we should disable the buttons
 */
void	GuiderDialog::capture() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "capture request");
	// construct an exposure object for the full image
	Astro::Exposure	exposure = _guider->getExposure();
	Astro::Ccd_var	ccd = _guider->getCcd();
	Astro::CcdInfo_var	info = ccd->getInfo();
	exposure.frame.origin.x = 0;
	exposure.frame.origin.y = 0;
	exposure.frame.size = info->size;
	exposure.shutter = Astro::SHUTTER_OPEN;

	// cleare the statistics
	ui->maxField->setText("");
	ui->minField->setText("");
	ui->meanField->setText("");

	// send the exposure request to the CCD
	ccd->startExposure(exposure);

	// wait for the exposure to complete
	while (ccd->exposureStatus() != Astro::EXPOSURE_EXPOSED) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "image not yet exposed");
		usleep(100000);
	}

	// retrieve an image from the server
	Astro::Image_var	image;
	try {
		image = ccd->getImage();
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "failure to receive new image");
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image retrieved");

	// get image size
	Astro::ImageSize	size = image->size();
	ui->scrollAreaWidgetContents->setFixedSize(size.width, size.height);

	// Variables for maximum/minimum/mean
	image_statistics	stats;

	// convert the image
	QPixmap	pixmap = image2pixmap(image, stats);

	// display the pixmap
	ui->imageLabel->setFixedSize(size.width, size.height);
	ui->imageLabel->setPixmap(pixmap);

	// update the statistics fields
	ui->maxField->setText(tr("%1").arg(stats.max));
	ui->minField->setText(tr("%1").arg(stats.min));
	ui->meanField->setText(tr("%1").arg(stats.mean));
}

/**
 * \brief Initiate calibration
 */
void	GuiderDialog::calibrate() {
	// action depends on current state
	switch (_guider->getState()) {
	case Astro::Guider::GUIDER_IDLE:
	case Astro::Guider::GUIDER_CALIBRATED:
		_guider->startCalibration(ui->focallengthSpinbox->value() / 1000.);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration started");
		break;
	case Astro::Guider::GUIDER_CALIBRATING:
		_guider->cancelCalibration();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cancel calibration");
		break;
	case Astro::Guider::GUIDER_GUIDING:
	case Astro::Guider::GUIDER_UNCONFIGURED:
		debug(LOG_ERR, DEBUG_LOG, 0,
			"cannot process calibration request in this state");
		break;
	}
}

/**
 * \brief
 */
void	GuiderDialog::guide() {
	switch (_guider->getState()) {
	case Astro::Guider::GUIDER_CALIBRATED:
		_guider->startGuiding(ui->guideintervalSpinbox->value());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "guiding started");
		break;
	case Astro::Guider::GUIDER_GUIDING:
		_guider->stopGuiding();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "guiding stopped");
		break;
	case Astro::Guider::GUIDER_IDLE:
	case Astro::Guider::GUIDER_CALIBRATING:
	case Astro::Guider::GUIDER_UNCONFIGURED:
		debug(LOG_ERR, DEBUG_LOG, 0,
			"cannot process calibration request in this state");
		break;
	}
}

/**
 * \brief method called when the monitor open button is clicked
 */
void	GuiderDialog::monitor() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "open monitor");
	// First find out whether we have a child that is a dialog of this
	// type. If so, we just bring it into the foreground

	// Ok, seems we have no previously created dialog, so we create one
	GuiderMonitorDialog	*monitordialog = new GuiderMonitorDialog(_guider, this);
	monitordialog->show();
}

/**
 * \brief handle mouse press events
 */
void	GuiderDialog::mousePressEvent(QMouseEvent *event) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "mouse press event: %d, %d",
		event->x(), event->y());
	QPoint	mousepos = ui->imageLabel->mapFrom(this, event->pos());
	if (!ui->imageLabel->rect().contains(mousepos)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "mouse press not insided image");
		return;
	}

	// check that we have an image
	if (NULL == ui->imageLabel->pixmap()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no image, ignoring event");
		return;
	}

	// determine mouse position
	debug(LOG_DEBUG, DEBUG_LOG, 0, "mouse position: %d, %d",
		mousepos.x(), mousepos.y());
	Astro::Point	p;
	p.x = mousepos.x();
	p.y = mousepos.y();

	Astro::Ccd_var	ccd = _guider->getCcd();
	Astro::CcdInfo_var	info = ccd->getInfo();
	p.y = info->size.height - 1 - p.y;

	if (p.x < 0) { p.x = 0; }
	if (p.x >= info->size.width) { p.x = info->size.width - 1; }
	if (p.y < 0) { p.y = 0; }
	if (p.y >= info->size.height) { p.y = info->size.height - 1; }

	_guider->setStar(p);
	setStar(_guider->getStar());

	// find size of the rectangle
	int	l = ui->sizeSpinbox->value();

	// get the exposure
	Astro::Exposure	exposure = _guider->getExposure();
	exposure.frame.size.width = 2 * l;
	exposure.frame.size.height = 2 * l;

	// compute lower left origin of the image rectangle, ensure it
	// stays inside the ccd area
	exposure.frame.origin.x = p.x - l;
	if (exposure.frame.origin.x < 0) {
		exposure.frame.origin.x = 0;
	}
	exposure.frame.origin.y = p.y - l;
	if (exposure.frame.origin.y < 0) {
		exposure.frame.origin.y = 0;
	}

	// ensure the it does not leave the ccd rectangle at the right or the top
	if ((exposure.frame.origin.x + exposure.frame.size.width)
		> info->size.width) {
		exposure.frame.origin.x
			= info->size.width - exposure.frame.size.width;
	}
	if ((exposure.frame.origin.y + exposure.frame.size.height)
		> info->size.height) {
		exposure.frame.origin.y
			= info->size.height - exposure.frame.size.height;
	}
	_guider->setExposure(exposure);
}

#if 0
/**
 * \brief What to do at each timer tick
 */
void	GuiderDialog::tick() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Timer tick");

	// handle most recent image
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "try most recent image");
		Astro::Image_var	image = _guider->mostRecentImage();
		image_statistics	stats;
		QPixmap	pixmap = image2pixmap(image, stats);
		ui->guideimageLabel->setPixmap(pixmap);
		image->remove();
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no most recent image: %s",
			x.what());
	} catch (...) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"no most recent image, unknown exception");
	}

	// update guider state
	try {
		setGuiderState(_guider->getState());
	} catch (...) { }
}
#endif

/**
 * \brief convert a CORBA image into a QPixmap
 *
 * As side effect, this method accumulates statistics data in the stats
 * argument. The method can handle byte or unsigned short data images.
 * \param image	Image object reference.
 * \param stats	Statistics about the image
 */
QPixmap	GuiderDialog::image2pixmap(Astro::Image_var image, image_statistics& stats) {
	Astro::ImageSize	size = image->size();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image of size %dx%d",
		size.width, size.height);
	QImage	qimage(size.width, size.height, QImage::Format_RGB32);

	// narrow down to a ShortImage
	Astro::ShortImage_var	shortimage = Astro::ShortImage::_narrow(image);
	if (!CORBA::is_nil(shortimage)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "got a short image");
		Astro::ShortSequence_var	imagedata
			= shortimage->getShorts();

		// find maximum, minimum and mean value
		for (unsigned int i = 0; i < imagedata->length(); i++) {
			double	v = imagedata[i];
			stats.add(v);
		}

		// convert the image to pixmap to display in the image label
		for (int x = 0; x < size.width; x++) {
			for (int y = 0; y < size.height; y++) {
				unsigned char	v 
					= imagedata[x + size.width * y] >> 8;
				unsigned long	value = (0xff000000) | (v << 16) | (v << 8) | v;
				qimage.setPixel(x, size.height - 1 - y, value);
			}
		}
	}

	Astro::ByteImage_var	byteimage = Astro::ByteImage::_narrow(image);
	if (!CORBA::is_nil(byteimage)) {
		Astro::ByteImage::ByteSequence_var	imagedata
			= byteimage->getBytes();

		// find maximum, minimum and mean value
		for (unsigned int i = 0; i < imagedata->length(); i++) {
			double	value = imagedata[i];
			stats.add(value);
		}

		// convert the image to pixmap to display in the image label
		for (int x = 0; x < size.width; x++) {
			for (int y = 0; y < size.height; y++) {
				unsigned char	v = imagedata[x + size.width * y];
				unsigned long	value = (0xff000000) | (v << 16) | (v << 8) | v;
				qimage.setPixel(x, size.height - 1 - y, value);
			}
		}
	}

	// convert data into an image
	QPixmap	pixmap(size.width, size.height);
	pixmap.convertFromImage(qimage);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image of size %d x %d created",
		size.width, size.height);
	return pixmap;

}

//////////////////////////////////////////////////////////////////////
// image_statistics implementation
//////////////////////////////////////////////////////////////////////
image_statistics::image_statistics() {
	min = 65535;
	max = 0;
	mean = sum = 0;
	count = 0;
}

void	image_statistics::add(double value) {
	if (value < min) {
		min = value;
	}
	if (value > max) {
		max = value;
	}
	sum += value;
	count++;
	mean = sum / count;
}
