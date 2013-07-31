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
#include <AstroIO.h>
#include <QThread>
#include <QFileDialog>
#include <QMessageBox>
#include "ExposureWorker.h"
#include <sys/time.h>
#include <AstroCalibration.h>
#include <AstroInterpolation.h>

using namespace astro;
using namespace astro::image;
using namespace astro::io;
using namespace astro::calibration;
using namespace astro::interpolation;

static double	nowtime() {
	struct timeval	now;
	gettimeofday(&now, NULL);
	return now.tv_sec + 0.000001 * now.tv_usec;
}

#define	EXPOSURE_MIN	0.001

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

	// make progress bar invisible
	ui->captureProgressBar->hide();
	ui->captureProgressBar->setMinimum(0);

	// create the timer, but don't start i
	timer = new QTimer();
	timer->setInterval(100);
	connect(timer, SIGNAL(timeout()), this, SLOT(timer_timeout()));

	// add menus
	fileMenu = menuBar()->addMenu("&File");
	QAction	*saveAction = fileMenu->addAction("&Save ...", this,
		SLOT(fileSaveAs()));
	saveAction->setShortcut(QKeySequence::Save);
}

/**
 * \brief destroy the CaptureWindow
 */
CaptureWindow::~CaptureWindow()
{
    delete ui;
    delete timer;
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
	exposure = ui->exposureWidget->getExposure();
	int	maxprogress = 100 * exposure.exposuretime;
	ui->captureProgressBar->setMaximum(maxprogress);
	exposurestart = nowtime();
	QThread	*thread = new QThread();
	ExposureWorker	*worker = new ExposureWorker(ccd, exposure, this);
	worker->moveToThread(thread);
	// when the thread is read, start the process method in the worker
	connect(thread, SIGNAL(started()), worker, SLOT(process()));
	// when the worker is done, hand over image to this class
	connect(worker, SIGNAL(finished()), this, SLOT(finished()));
	// when the worker signals finish, quit the thread
	connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
	// when the worker signals finish, mark it for deletetion
	connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
	// when the thread signals finished, mark it for deleteion
	connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
	thread->start();
	if (exposure.exposuretime > 1) {
		ui->captureButton->hide();
		ui->captureProgressBar->show();
		// start a timer
		timer->start();
	}
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
	ui->sizeinfoField->setText(QString(image->size().toString().c_str()));

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

	// just for debugging
	displayconverter.setMinpixel(image->minimum());
	displayconverter.setMaxpixel(image->maximum());

	// compute the image scale
	int	scaleitem = ui->scaleCombobox->currentIndex();
	imagescale = 0.25;
	while (scaleitem-- > 0) {
		imagescale *= 2;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "scale: %f", imagescale);

	// find out whether color display is expected: if the image is
	// color, we obviously display the color image. If we have a
	// monochrome image, we have demosaicing requested, then the
	// demosaiced image should be displayed
	bool	colordisplay = false;
	if (isColorImage(image)) {
		colordisplay = true;
	}
	if ((image->getMosaicType().isMosaic()) && (ui->demosaicCheckbox->isChecked())) {
		colordisplay = true;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "color display: %s",
		(colordisplay) ? "YES" : "NO");
	displayconverter.setColor(colordisplay);

	// apply the display conversion to the image
	Image<RGB<unsigned char> >	*imptr = NULL;
	if (image->getMosaicType().isMosaic()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "display demosaiced image");
		imptr = displayconverter(demosaicedimage);
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "display raw image");
		imptr = displayconverter(image);
	}
	ImagePtr	displayimage(imptr);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "converted image size: %s",
		displayimage->size().toString().c_str());

	// convert the image to 
	int32_t	*data = (int32_t *)calloc(imptr->size().getPixels(), sizeof(int32_t));

	for (unsigned int x = 0; x < imptr->size().width(); x++) {
		for (unsigned int y = 0; y < imptr->size().height(); y++) {
			RGB<unsigned char>	v = imptr->pixel(x, y);
			data[x + (imptr->size().height() - 1 - y) * imptr->size().width()]
				= (v.R << 16) | (v.G << 8) | v.B;
		}
	}
	QImage	qimage((unsigned char *)data, imptr->size().width(),
		imptr->size().height(), QImage::Format_RGB32);
	QPixmap	pixmap = QPixmap::fromImage(qimage);

	// display in the image area
	QSize	displaysize(imagescale * image->size().width(),
			imagescale * image->size().height());
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

	// create the Imager
	Imager	imager;
	imager.setDark(dark);
	imager.setFlat(flat);

	// get the rectangle for the correctors
	ImageRectangle	frame = image->getFrame();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new image has frame: %s",
		frame.toString().c_str());

	// perform calibration
	if (ui->darksubtractCheckbox->isChecked()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"dark correct with dark of size %s",
			dark->size().toString().c_str());
		imager.setDarksubtract(true);
	}

	if (ui->flatdivideCheckbox->isChecked()) {
		imager.setFlatsubtract(true);
	}

	if (ui->badpixelsCheckBox->isChecked()) {
		imager.setInterpolate(true);
	}

	imager(image);

	// demosaic the image
	ui->demosaicCheckbox->setEnabled(image->getMosaicType().isMosaic());
	if (image->getMosaicType().isMosaic()) {
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
	int	xoffset = (size.width() - imagescale * image->size().width()) / 2;
	if (xoffset < 0) {
		xoffset = 0;
	}
	int	yoffset = (size.height() - imagescale * image->size().height()) / 2;
	if (yoffset < 0) {
		yoffset = 0;
	}

	int	x = (mousepos.x() - xoffset) / imagescale;
	int	y = image->size().height()
			- (mousepos.y() - yoffset) / imagescale;
	
	// normalize into the image rectangle
	if (x < 0) {
		x = 0;
	}
	if (x >= image->size().width()) {
		x = image->size().width() - 1;
	}
	if (y < 0) {
		y = 0;
	}
	if (y >= image->size().height()) {
		y = image->size().height() - 1;
	}

	// access the value
	ImagePoint	p(x, y);
	double	v = astro::image::filter::rawvalue(image, p);
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "raw value: %f", v);
	QString	al(stringprintf("Value at (%d,%d):", p.x(), p.y()).c_str());
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

/**
 * \brief
 */ 
void	CaptureWindow::finished() {
	ui->captureProgressBar->hide();
	ui->captureButton->show();
	setImage(newimage);
	timer->stop();
}

/**
 * \brief method used
 */
void	CaptureWindow::newImage(ImagePtr _newimage) {
	newimage = _newimage;
}

/**
 * \brief
 */
void	CaptureWindow::timer_timeout() {
	// compute time passed since start
	int	progress = 100 * (nowtime() - exposurestart);
	ui->captureProgressBar->setValue(progress);
}

/**
 * \brief Slot that saves the current image as a file
 */
bool	CaptureWindow::fileSaveAs() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "save file");
	QString	filename = QFileDialog::getSaveFileName();
	std::string	filenamestring
		= filename.toUtf8().constData();
	ui->statusbar->showMessage(QString("Save to '%1'").arg(filename));
	if (filename.isEmpty()) {
		return false;
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "writing file '%s'",
		filenamestring.c_str());
	unlink(filenamestring.c_str());
	FITSout	out(filenamestring);
	out.write(image);
	ui->statusbar->showMessage(QString("Save to '%1'").arg(filename));
	return true;
}

/**
 * \brief Slot for opening a dark file
 */
void	CaptureWindow::openDarkfile() {
	darkfilename = QFileDialog::getOpenFileName();
	if (darkfilename.isEmpty()) {
		return;
	}
	std::string	darkfilenamestring(darkfilename.toUtf8().constData());
	try {
		FITSin	in(darkfilenamestring);
		ImagePtr	newdark = in.read();
		if (newdark->size() != ccd->getInfo().getSize()) {
			QMessageBox::warning(this,
				QString("Cannot use dark image"),
				QString("The dark file '%1' cannot be used, because it does not match the CCD size").arg(darkfilename));
			return;
		}
		dark = newdark;
		ui->darksubtractCheckbox->setEnabled(true);
		ui->badpixelsCheckBox->setEnabled(true);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "dark of size %s read",
			dark->size().toString().c_str());
	} catch (std::exception& x) {
		std::string	msg = 
			stringprintf("cannot open file: '%s'", x.what());
		ui->statusbar->showMessage(QString(msg.c_str()));
	}
	ui->darkField->setText(darkfilename);
}

/**
 * \brief Slot for opening a flat file
 */
void	CaptureWindow::openFlatfile() {
	flatfilename = QFileDialog::getOpenFileName();
	if (flatfilename.isEmpty()) {
		return;
	}
	std::string	flatfilenamestring(flatfilename.toUtf8().constData());
	try {
		FITSin	in(flatfilenamestring);
		ImagePtr	newflat = in.read();
		if (newflat->size() != ccd->getInfo().getSize()) {
			QMessageBox::warning(this,
				QString("Cannot use flat image"),
				QString("The flat file '%1' cannot be used, because it does not match the CCD size").arg(flatfilename));
			return;
		}
		flat = newflat;
		ui->flatdivideCheckbox->setEnabled(true);
	} catch (std::exception& x) {
		std::string	msg = 
			stringprintf("cannot open file: '%s'", x.what());
		ui->statusbar->showMessage(QString(msg.c_str()));
	}
	ui->flatField->setText(flatfilename);
}

/**
 * \brief Slot when bad pixels are toggled
 */
void	CaptureWindow::badpixelsToggled(bool state) {
	if (state) {
		ui->badpixelsCheckBox->setText(QString("enabled: interpolate"));
	} else {
		ui->badpixelsCheckBox->setText(QString("disabled: set to 0"));
	}
}
