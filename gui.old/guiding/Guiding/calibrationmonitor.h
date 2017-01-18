#ifndef CALIBRATIONMONITOR_H
#define CALIBRATIONMONITOR_H

#include <QWidget>
#include <guider.hh>
#include "connectiondialog.h"

namespace Ui {
class CalibrationMonitor;
}

namespace calibrationmonitor {

class CalibrationMonitor_impl;

} // namespace calibrationmonitor

class CalibrationMonitor : public QWidget
{
	Q_OBJECT

	Astro::Guider_var	_guider;
	Astro::Calibration_var	_calibration;
	int	monitorid;
	calibrationmonitor::CalibrationMonitor_impl	*cm_impl;
	void	registerServants();
	void	unregisterServants();
public:
	explicit CalibrationMonitor(Astro::Guider_var guider,
			QWidget *parent = 0);
	~CalibrationMonitor();

private:
	void	updateWidgets(int i, const Astro::CalibrationPoint& point);
public:
	void	addPoint(const Astro::CalibrationPoint& point);
	void	addCalibration(const Astro::Calibration_var calibration);
	void	stopCalibration();
public slots:
	void	rereadCalibration();
	void	display();
signals:
	void	pointUpdated();
	void	stopSignal();

private:
	Ui::CalibrationMonitor *ui;
};

namespace calibrationmonitor {

/**
 * \brief Calibration Monitor for the Qt CalibrationWidget
 */
class CalibrationMonitor_impl : public POA_Astro::CalibrationMonitor {
	::CalibrationMonitor&	_calibrationmonitor;
public:
	CalibrationMonitor_impl(::CalibrationMonitor& calibrationwidget);
	virtual ~CalibrationMonitor_impl();
	virtual void	update(const ::Astro::CalibrationPoint& cp);
	virtual void	stop();
};

}

#endif // CALIBRATIONMONITOR_H
