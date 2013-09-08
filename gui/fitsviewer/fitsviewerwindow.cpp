#include "fitsviewerwindow.h"
#include "ui_fitsviewerwindow.h"
#include <AstroViewer.h>
#include <AstroDebug.h>
#include <QWidget.h>

using namespace astro::image;

FITSViewerWindow::FITSViewerWindow(QWidget *parent,
	const std::string& filename) :
    QMainWindow(parent),
    ui(new Ui::FITSViewerWindow),
    viewer(filename)
{
    ui->setupUi(this);
    setWindowTitle(QString(filename.c_str()));
    debug(LOG_DEBUG, DEBUG_LOG, 0, "ui for FITSViewerWindow created");
    ui->histogramWidget->setHistograms(viewer.histograms());
    unsigned int	width = viewer.previewsize().width();
    unsigned int	height = viewer.previewsize().height();
    debug(LOG_DEBUG, DEBUG_LOG, 0, "preview size: %u x %u", width, height);
    viewer.previewwidth(ui->previewLabel->width());
    int	min = viewer.min();
    int max = viewer.max();
    ui->minSpinner->setValue(min);
    ui->maxSpinner->setValue(max);
    RGB<float>	colorcorrection = viewer.colorcorrection();
    ui->redSpinBox->setValue(colorcorrection.R);
    ui->greenSpinBox->setValue(colorcorrection.G);
    ui->blueSpinBox->setValue(colorcorrection.B);
    update();
    previewupdate();
    backgroundupdate();
    debug(LOG_DEBUG, DEBUG_LOG, 0, "FITSViewerWindow constructor complete");
}

FITSViewerWindow::~FITSViewerWindow()
{
    delete ui;
}

void	FITSViewerWindow::update() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "main update()");
	// copy the image data to a pixmap
	unsigned int	width = viewer.size().width();
	unsigned int	height = viewer.size().height();
	QImage	qimage((unsigned char *)viewer.imagedata(), width, height,
		QImage::Format_RGB32);
	QPixmap	pixmap = QPixmap::fromImage(qimage);
	ui->imageLabel->setPixmap(pixmap);
	ui->histogramWidget->setMinmark(viewer.min());
	ui->histogramWidget->setMaxmark(viewer.max());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "main update() ends");
}

void	FITSViewerWindow::previewupdate() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "previewupdate()");
	// copy the image data to a pixmap
	unsigned int	width = viewer.previewsize().width();
	unsigned int	height = viewer.previewsize().height();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "preview update %u x %u",
		width, height);
	QImage	qimage((unsigned char *)viewer.previewdata(), width, height,
		QImage::Format_RGB32);
	QPixmap	pixmap = QPixmap::fromImage(qimage);
	ui->previewLabel->setPixmap(pixmap);
	ui->histogramWidget->setMinmark(viewer.min());
	ui->histogramWidget->setMaxmark(viewer.max());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "previewupdate() ends");
}

void	FITSViewerWindow::backgroundupdate() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "backgroundupdate()");
	// copy the image data to a pixmap
	unsigned int	width = viewer.backgroundsize().width();
	unsigned int	height = viewer.backgroundsize().height();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "background update %u x %u",
		width, height);
	QImage	qimage((unsigned char *)viewer.backgrounddata(), width, height,
		QImage::Format_RGB32);
	QPixmap	pixmap = QPixmap::fromImage(qimage);
	ui->backgroundLabel->setPixmap(pixmap);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "backgroundupdate() ends");
}

void	FITSViewerWindow::gammaChanged(int value) {
	viewer.gamma(value / 100.);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "gamma changed: %f", viewer.gamma());
	viewer.previewupdate();
	previewupdate();
}

void	FITSViewerWindow::gradientChanged(int state) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new gradient state: %s",
		(state) ? "enabled" : "disabled");
	viewer.gradientEnabled((state) ? true : false);
	viewer.previewupdate();
	viewer.backgroundupdate();
	backgroundupdate();
	previewupdate();
}

void	FITSViewerWindow::backgroundChanged(int state) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new background state: %s",
		(state) ? "subtract" : "don't subtract");
	viewer.backgroundEnabled((state) ? true : false);
	viewer.previewupdate();
	viewer.backgroundupdate();
	backgroundupdate();
	previewupdate();
}

void	FITSViewerWindow::rangeChanged(int value) {
	unsigned int	min = ui->minSpinner->value();
	unsigned int	max = ui->maxSpinner->value();
	viewer.setRange(min, max);
	viewer.previewupdate();
	ui->histogramWidget->updateGeometry();
	previewupdate();
}

void	FITSViewerWindow::doUpdate() {
	viewer.update();
	update();
}

void	FITSViewerWindow::saturationChanged(int value) {
	viewer.saturation(value / 100.);
	viewer.previewupdate();
	previewupdate();
}

void	FITSViewerWindow::colorcorrectionChanged(double value) {
	float	R = ui->redSpinBox->value();
	float	G = ui->greenSpinBox->value();
	float	B = ui->blueSpinBox->value();
	viewer.colorcorrection(RGB<float>(R, G, B));
	viewer.previewupdate();
	previewupdate();
}
