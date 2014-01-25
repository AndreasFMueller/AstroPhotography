/*
 * calibrationpointwidget.h -- Widget to display the calibration points
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _calibrationpointwidget_h
#define _calibrationpointwidget_h

#include <QWidget.h>
#include <guider.hh>

/**
 * \brief Widget to display the calibrations point of a calibration object
 */
class CalibrationPointWidget : public QWidget {
	Q_OBJECT
public:

	typedef std::pair<double, double>	point_t;
	typedef std::vector<point_t>	points_t;
private:
	points_t	points;
	void	drawPoints();
public:
	bool	circle;
	bool	grid;
	QColor	color;
	void	addPoint(const point_t& point);
	void	addPoint(const Astro::Point& point);
	void	clear();
	void	paintEvent(QPaintEvent *event);

public:
	explicit CalibrationPointWidget(QWidget *parent = 0);
	virtual ~CalibrationPointWidget();

public slots:

private slots:

};

#endif /* _calibrationpointwidget_h */
