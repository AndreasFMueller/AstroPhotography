/*
 * capturewindow.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "capturewindow.h"
#include "ui_capturewindow.h"
#include <stdlib.h>
#include <AstroDisplay.h>
#include <AstroFilterfunc.h>
#include <AstroFormat.h>
#include <AstroDemosaic.h>

using namespace astro;
using namespace astro::image;

/**
 * \brief construct a CaptureWindow
 */
CaptureWindow::CaptureWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CaptureWindow)
{
	// setup othe UI elements
	ui->setupUi(this);

	// initialize the exposure object to reasonable defaults
	exposure.exposuretime = 0.001;

	// create a background for the image
	QPixmap	background(640,480);
	ui->imageLabel->setPixmap(background);

	// populate the scale items
	ui->scaleCombobox->addItem(QString("25%"));
	ui->scaleCombobox->addItem(QString("50%"));
	ui->scaleCombobox->addItem(QString("100%"));
	ui->scaleCombobox->addItem(QString("200%"));
	ui->scaleCombobox->addItem(QString("400%"));
	ui->scaleCombobox->setCurrentIndex(2);
}

/**
 * \brief destroy the CaptureWindow
 */
CaptureWindow::~CaptureWindow()
{
    delete ui;
}

/**
 * \brief Build a camera/ccd description string
 *
 * The window title of the CaptureWindow is composed of the camera and
 * CCD information. This method constructs the string in a uniform way.
 */
QString	CaptureWindow::getCameraTitle() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "getting camera title");
	std::string	cameraname
		= (camera) ? camera->getName() : std::string("(unknown)");
	std::string	ccdname
		= (ccd) ?  (ccd->getInfo().name
				+ std::string(" (")
				+ ccd->getInfo().size.toString()
				+ std::string(")"))
			: std::string("(unknown)");

	std::string	titlestring
		= std::string("Camera: ") + cameraname
		+ std::string(", CCD: ") + ccdname;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera name: %s", titlestring.c_str());
	return QString(titlestring.c_str());
}

/**
 * \brief Set the camera
 */
void	CaptureWindow::setCamera(CameraPtr _camera) {
	camera = _camera;
	setWindowTitle(getCameraTitle());
}

/**
 * \brief Set the CCD
 */
void	CaptureWindow::setCcd(CcdPtr _ccd) {
	ccd = _ccd;

	ui->exposureWidget->setCcd(ccd);

	// update the window title
	setWindowTitle(getCameraTitle());

	// set the frame size 
	exposure.frame = ccd->getInfo().getFrame();
	ui->exposureWidget->setExposure(exposure);

	// find out whether this CCD has a cooler, disable the cooler box
	// if it hasn't
	CoolerPtr	cooler;
	try {
		ccd->getCooler();
		if (!cooler) {
			ui->coolerBox->setEnabled(false);
		}
	} catch (std::exception& x) {
		ui->coolerBox->setEnabled(false);
	}
}

/**
 * \brief Slot called when a capture is started
 */
void	CaptureWindow::startCapture() {
	ui->statusbar->showMessage(QString("capturing new image"));
	// XXX This implementation is current synchronous, which means
	//     that long exposures completely block the UI. This should
	//     be change so that a separate thread is performing the
	//     capture
	debug(LOG_DEBUG, DEBUG_LOG, 0, "startCapture called");
	exposure = ui->exposureWidget->getExposure();
	ccd->startExposure(exposure);
	setImage(ccd->getImage());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got image");
}

/**
 * \brief Redisplay the image, possibly with modified 
 */
void	CaptureWindow::redisplayImage() {
	bool	ok;

	// get information from the new image
	double	maxvalue = astro::image::filter::max(image);
	ui->maximumvalueField->setText(QString().setNum(maxvalue, 'f', 0));
	double	minvalue = astro::image::filter::min(image);
	ui->minimumvalueField->setText(QString().setNum(minvalue, 'f', 0));
	double	meanvalue = astro::image::filter::mean(image);
	ui->meanvalueField->setText(QString().setNum(meanvalue, 'f', 1));
	ui->sizeinfoField->setText(QString(image->size.toString().c_str()));

	// convert image into a Pixmap
	DisplayConverter	displayconverter;

	// set the pixel value scaling parameters. If they are not set in
	// the gui, then we take the maximum and minimum values from the
	// statistics
	int	minpixel = ui->valueintervalminField->text().toInt(&ok);
	if (ok) {
		displayconverter.setMinpixel(0);
	} else {
		displayconverter.setMinpixel(minvalue);
	}
	int	maxpixel = ui->valueintervalminField->text().toInt(&ok);
	if (ok) {
		if (maxpixel <= minpixel) {
			maxpixel = minpixel + 1;
			ui->valueintervalminField->setText(QString().setNum(maxpixel));
		}
		displayconverter.setMaxpixel(maxpixel);
	} else {
		displayconverter.setMaxpixel(maxvalue);
	}

	// compute the image scale
	int	scaleitem = ui->scaleCombobox->currentIndex();
	imagescale = 0.25;
	while (scaleitem-- > 0) {
		imagescale *= 2;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "scale: %f", imagescale);

	// apply the display conversion to the image
	Image<RGB<unsigned char> >	*imptr
		= displayconverter(image);
	ImagePtr	displayimage(imptr);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "converted image size: %s",
		displayimage->size.toString().c_str());

	// convert the image to 
	int32_t	*data = (int32_t *)calloc(imptr->size.getPixels(), sizeof(int32_t));

	for (unsigned int x = 0; x < imptr->size.getWidth(); x++) {
		for (unsigned int y = 0; y < imptr->size.getHeight(); y++) {
			RGB<unsigned char>	v = imptr->pixel(x, y);
			data[x + (imptr->size.getHeight() - 1 - y) * imptr->size.getWidth()]
				= (v.R << 16) | (v.G << 8) | v.B;
		}
	}
	QImage	qimage((unsigned char *)data, imptr->size.getWidth(),
		imptr->size.getHeight(), QImage::Format_RGB32);
	QPixmap	pixmap = QPixmap::fromImage(qimage);

	// display in the image area
	QSize	displaysize(imagescale * image->size.getWidth(),
			imagescale * image->size.getHeight());
	ui->imageLabel->setPixmap(pixmap.scaled(displaysize,
		Qt::KeepAspectRatio));

	free(data);
}

/**
 * \brief update the GUI when an image has been retrieved
 */
void	CaptureWindow::setImage(ImagePtr newimage) {
	ui->statusbar->showMessage(QString("new image captured"));
	image = newimage;
	ui->demosaicCheckbox->setEnabled(image->isMosaic());
	if (image->isMosaic()) {
		demosaicedimage = demosaic_bilinear(image);
	}
	redisplayImage();
}

/**
 * \brief Filter for Mouse events
 *
 * This allows us to capture events when we drag over the image, and display
 * current values.
 */
void CaptureWindow::mouseMoveEvent(QMouseEvent* event) {
	if (!image) {
		return;
	}
	// turn coordinates into image point
	QPoint	mousepos = ui->imageLabel->mapFrom(this, event->pos());

	// compute imageLabel size
	QSize	size = ui->imageLabel->size();
	int	xoffset = (size.width() - imagescale * image->size.getWidth()) / 2;
	if (xoffset < 0) {
		xoffset = 0;
	}
	int	yoffset = (size.height() - imagescale * image->size.getHeight()) / 2;
	if (yoffset < 0) {
		yoffset = 0;
	}

	int	x = (mousepos.x() - xoffset) / imagescale;
	int	y = image->size.getHeight()
			- (mousepos.y() - yoffset) / imagescale;
	
	// normalize into the image rectangle
	if (x < 0) {
		x = 0;
	}
	if (x >= image->size.getWidth()) {
		x = image->size.getWidth() - 1;
	}
	if (y < 0) {
		y = 0;
	}
	if (y >= image->size.getHeight()) {
		y = image->size.getHeight() - 1;
	}

	// access the value
	ImagePoint	p(x, y);
	double	v = astro::image::filter::rawvalue(image, p);
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "raw value: %f", v);
	QString	al(stringprintf("Value at (%d,%d):", p.x, p.y).c_str());
	ui->valueatLabel->setText(al);
	ui->valueatField->setText(QString().setNum(v, 'f', 0));
}

/**
 * \brief Slot called when the scale changes
 */
void	CaptureWindow::scaleChanged(int item) {
	if (image) {
		ui->statusbar->showMessage(
			QString("scaling image to ") +
			ui->scaleCombobox->currentText());
		redisplayImage();
		ui->statusbar->showMessage(
			QString("image scaled to ") +
			ui->scaleCombobox->currentText());
	}
}

