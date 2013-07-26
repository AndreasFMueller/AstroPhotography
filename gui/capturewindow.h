#ifndef CAPTUREWINDOW_H
#define CAPTUREWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
#include <AstroCamera.h>

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
	ImagePtr	image; // most recent image
	ImagePtr	demosaicedimage;
	double	imagescale;
	double	timeprevious;
	bool	timechange;
    
public:
    explicit CaptureWindow(QWidget *parent = 0);
    ~CaptureWindow();

	QString	getCameraTitle();
	void	setCamera(CameraPtr camera);
	void	setCcd(CcdPtr ccd);

	Exposure	getExposure();
	void	setExposure(const Exposure& exposure);

	void	setImage(ImagePtr newimage);
	void	redisplayImage();

	virtual void mouseMoveEvent(QMouseEvent* event);
    
private:
    Ui::CaptureWindow *ui;

private slots:
	void	scaleChanged(int item);
	void	startCapture();
	void	subframeToggled(bool state);
	void	timeChanged(double value);
};

#endif // CAPTUREWINDOW_H
