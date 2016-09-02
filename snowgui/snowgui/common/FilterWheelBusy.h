/*
 * FilterWheelBusy.h
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _FilterWheelBusy_h
#define _FilterWheelBusy_h

#include <QWidget>
#include <QTimer>

namespace snowgui {

/**
 * \brief Indicator class the shows how the filterwheel rotates
 */
class FilterWheelBusy : public QWidget {
	Q_OBJECT

	QTimer	*timer;
	int	_nfilters;
	double	_starttime;
	double	_angle;
public:
	explicit FilterWheelBusy(QWidget *parent = NULL);
	virtual ~FilterWheelBusy();
	void	start();
	void	stop();
	int	nfilters() const { return _nfilters; }
	void	nfilters(int n) { _nfilters = n; }
	void	draw();
	void	paintEvent(QPaintEvent *event);
	void	position(int p);
public slots:
	void	update();
};

} // namespace snowgui

#endif /* _FilterWheelBusy_h */
