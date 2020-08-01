/*
 * BusyWidget.h -- transparent busy indicator widget for StarChart
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswi
 */
#ifndef _BusyWidget_h 
#define _BusyWidget_h

#include <QWidget>
#include <QTimer>
#include <AstroUtils.h>

namespace snowgui {

/**
 * \brief Class to show a busy indicator
 */
class BusyWidget : public QWidget {
	Q_OBJECT
	QTimer	_timer;
	void	draw();
	astro::Timer	_start;
public:
	explicit BusyWidget(QWidget *parent = NULL);
	~BusyWidget();
protected:
	void	paintEvent(QPaintEvent *event);
public slots:
	void	update();
	void	start();
	void	stop();
};

} // namespace snowgui

#endif /* _BusyWidget_h */
