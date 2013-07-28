#ifndef CAPTUREWINDOW_H
#define CAPTUREWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
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
};

#endif // CAPTUREWINDOW_H
