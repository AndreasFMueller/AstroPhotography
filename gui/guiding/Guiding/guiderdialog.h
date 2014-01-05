#ifndef GUIDERDIALOG_H
#define GUIDERDIALOG_H

#include <guider.hh>

#include <QDialog>
#include <QMouseEvent>
#include <QTimer>

namespace Ui {
class GuiderDialog;
}

class image_statistics {
public:
	double	min;
	double	max;
	double	mean;
private:
	double	sum;
	unsigned long	count;
public:
	image_statistics();
	void	add(double value);
};

class GuiderDialog : public QDialog
{
	Q_OBJECT

	Astro::Guider_var	_guider;
	double	lastimageago;

public:
	explicit GuiderDialog(Astro::Guider_var guider, QWidget *parent = 0);
	~GuiderDialog();

	Astro::Guider_var	guider() { return _guider; }
	void	setExposure(const Astro::Exposure& exposure);
	void	setGuiderState(const Astro::Guider::GuiderState& guiderstate);
	void	setStar(const Astro::Point& star);
protected:
	void	mousePressEvent(QMouseEvent *event);

private:
	Ui::GuiderDialog *ui;
	QPixmap	image2pixmap(Astro::Image_var image, image_statistics& stats);

private slots:
	void	capture();
	void	calibrate();
	void	guide();
	void	exposuretime(double t);
	void	monitor();
};

#endif // GUIDERDIALOG_H
