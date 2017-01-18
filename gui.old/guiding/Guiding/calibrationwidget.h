/*
 * calibrationwidget.h -- Widget to display the calibration as two vectors
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _calibrationwidget_h
#define _calibrationwidget_h

#include <QWidget>
#include <guider.hh>

class CalibrationWidget : public QWidget {
	Q_OBJECT
private:
	void	draw();
public:
	typedef std::pair<double, double>	point_t;

private:
	point_t	ravector;
	point_t	decvector;
	point_t	driftvector;
	void	updateWidgets(const Astro::CalibrationPoint& point);
public:
	void	addCalibration(const Astro::Calibration& calibration);
	void	addPoint(const Astro::CalibrationPoint& point);
	void	paintEvent(QPaintEvent *event);
public:
	explicit CalibrationWidget(QWidget *parent = 0);
	virtual ~CalibrationWidget();
public slots:

private slots:
};

#endif /* _calibrationwidget_h */
