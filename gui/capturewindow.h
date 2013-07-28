#ifndef CAPTUREWINDOW_H
#define CAPTUREWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
#include <QTimer>
#include <AstroCamera.h>
#include <exposurewidget.h>

using namespace astro::camera;
using namespace astro::image;

namespace Ui {
class CaptureWindow;
}

class CaptureWindow : public QMainWindow
{
    Q_OBJECT

	CameraPtr	camera;
	CcdPtr	ccd;
	Exposure	exposure;
	ImagePtr	newimage;
	ImagePtr	image; // most recent image
	ImagePtr	demosaicedimage;
	double	imagescale;

	// timing and progress
	QTimer	*timer;
	double	exposurestart;

	// Menus
	QMenu	*fileMenu;

	// dark and flat files
	QString	darkfilename;
	ImagePtr	dark;
	QString	flatfilename;
	ImagePtr	flat;
    
public:
    explicit CaptureWindow(QWidget *parent = 0);
    ~CaptureWindow();

	QString	getCameraTitle();
	void	setCamera(CameraPtr camera);
	void	setCcd(CcdPtr ccd);

	void	setImage(ImagePtr newimage);
	void	redisplayImage();

	virtual void mouseMoveEvent(QMouseEvent* event);
	void	newImage(ImagePtr newimage);
    
private:
    Ui::CaptureWindow *ui;

private slots:
	void	scaleChanged(int item);
	void	startCapture();
	void	finished();
	void	timer_timeout();
	bool	fileSaveAs();
	void	openDarkfile();
	void	openFlatfile();
};

#endif // CAPTUREWINDOW_H
