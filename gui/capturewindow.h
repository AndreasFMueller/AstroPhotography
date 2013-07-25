#ifndef CAPTUREWINDOW_H
#define CAPTUREWINDOW_H

#include <QMainWindow>
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
    
public:
    explicit CaptureWindow(QWidget *parent = 0);
    ~CaptureWindow();

	QString	getCameraTitle();
	void	setCamera(CameraPtr camera);
	void	setCcd(CcdPtr ccd);
    
private:
    Ui::CaptureWindow *ui;

private slots:
	void	startCapture();
};

#endif // CAPTUREWINDOW_H
