#include "fitsviewerwindow.h"
#include "ui_fitsviewerwindow.h"
#include <AstroViewer.h>
#include <AstroDebug.h>

using namespace astro::image;

FITSViewerWindow::FITSViewerWindow(QWidget *parent, const std::string& filename) :
    QMainWindow(parent),
    ui(new Ui::FITSViewerWindow),
    viewer(filename)
{
    debug(LOG_DEBUG, DEBUG_LOG, 0, "setting up FITSViewerWindow");
    ui->setupUi(this);
    debug(LOG_DEBUG, DEBUG_LOG, 0, "ui for FITSViewerWindow created");
    update();
    debug(LOG_DEBUG, DEBUG_LOG, 0, "FITSViewerWindow constructor complete");
}

FITSViewerWindow::~FITSViewerWindow()
{
    delete ui;
}

void	FITSViewerWindow::update() {
	// copy the image data to a pixmap
	unsigned int	width = viewer.size().width();
	unsigned int	height = viewer.size().height();
	QImage	qimage((unsigned char *)viewer.imagedata(), width, height,
		QImage::Format_RGB32);
	QPixmap	pixmap = QPixmap::fromImage(qimage);
	ui->imageLabel->setPixmap(pixmap);
}
