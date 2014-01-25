/*
 * guiderwidget.h -- Main window for guider control
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef GUIDERWIDGET_H
#define GUIDERWIDGET_H

#include <QWidget>
#include <QTimer>
#include <guider.hh>
#include <QMouseEvent>

namespace Ui {
class GuiderWidget;
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

class GuiderWidget : public QWidget
{
	Q_OBJECT

	Astro::Guider_var	_guider;
	QTimer	*timer;
	double	lastimageago;

public:
	explicit GuiderWidget(Astro::Guider_var guider, QWidget *parent = 0);
	~GuiderWidget();

	Astro::Guider_var	guider() { return _guider; }
	void	setExposure(const Astro::Exposure& exposure);
	void	setGuiderState(const Astro::Guider::GuiderState& guiderstate);
	void	setStar(const Astro::Point& star);

private:
	Ui::GuiderWidget *ui;
	QPixmap	image2pixmap(Astro::Image_var image, image_statistics& stats);

protected:
	void	mousePressEvent(QMouseEvent *event);

private slots:
	void	capture();
	void	calibrate();
	void	guide();
	void	exposuretime(double t);
	void	monitor();
	void	calibration();
	void	tick();
};

#endif // GUIDERWIDGET_H
