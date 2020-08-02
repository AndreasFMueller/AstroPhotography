/*
 * HeartWidget.h
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _HeartWidget_h
#define _HeartWidget_h

#include <QWidget>
#include <QTimer>
#include <AstroUtils.h>

namespace snowgui {

class HeartWidget : public QWidget {
	Q_OBJECT
	QTimer	_timer;
	void	draw();
	astro::Timer	_start;
	double	_beattime;
	double	_interval;
	QColor	_color;
public:
	explicit HeartWidget(QWidget *parent = NULL);
	~HeartWidget();
	double	interval() const { return _interval; }
	void	interval(double i) { _interval = i; }
protected:
	void	paintEvent(QPaintEvent *event);
public slots:
	void	beat();
	void	update();
	void	dead();
};

} // namespace snowgui

#endif /* _HeartWidget_h */
