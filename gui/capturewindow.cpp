#include "capturewindow.h"
#include "ui_capturewindow.h"
#include <stdlib.h>

CaptureWindow::CaptureWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CaptureWindow)
{
    ui->setupUi(this);
	exposure.exposuretime = 0.001;

	QPixmap	background(640,480);
	ui->imageLabel->setPixmap(background);
}

CaptureWindow::~CaptureWindow()
{
    delete ui;
}

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

void	CaptureWindow::setCamera(CameraPtr _camera) {
	camera = _camera;
	setWindowTitle(getCameraTitle());
}

void	CaptureWindow::setCcd(CcdPtr _ccd) {
	ccd = _ccd;
	setWindowTitle(getCameraTitle());
	// read the binning modes and add the options to the combo box
	const BinningSet&	set = ccd->getInfo().binningmodes;
	std::set<Binning>::const_iterator	bi;
	for (bi = set.begin(); bi != set.end(); bi++) {
		ui->binningComboBox->addItem(QString(bi->toString().c_str()));
	}
	exposure.frame = ccd->getInfo().getFrame();
}

void	CaptureWindow::startCapture() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "startCapture called");
	ccd->startExposure(exposure);
	image = ccd->getImage();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got image");

	// convert image into a Pixmap
	Image<unsigned short>	*imptr
		= dynamic_cast<Image<unsigned short> *>(&*image);
	int32_t	*data = (int32_t *)calloc(imptr->size.pixels, sizeof(int32_t));
	for (unsigned int x = 0; x < imptr->size.width; x++) {
		for (unsigned int y = 0; y < imptr->size.height; y++) {
			unsigned char	v = imptr->pixel(x, y) >> 8;
			data[x + y * image->size.width]
				= (v << 16) | (v << 8) | v;
		}
	}
	QImage	qimage((unsigned char *)data, image->size.width, image->size.height,
		QImage::Format_RGB32);
	QPixmap	pixmap = QPixmap::fromImage(qimage);

	// display in the image area
	ui->imageLabel->setPixmap(pixmap);
}
