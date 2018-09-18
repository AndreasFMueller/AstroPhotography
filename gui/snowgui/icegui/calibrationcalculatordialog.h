/*
 * calibrationcalculatordialog.h
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil 
 */
#ifndef CALIBRATIONCALCULATORDIALOG_H
#define CALIBRATIONCALCULATORDIALOG_H

#include <QDialog>
#include <guider.h>
#include "calibrationwidget.h"

namespace snowgui {

namespace Ui {
	class calibrationcalculatordialog;
}

class calibrationcalculatordialog : public QDialog {
	Q_OBJECT

	snowstar::GuiderPrx		_guider;
	snowstar::GuiderFactoryPrx	_guiderfactory;

	calibrationwidget	*_calibrationwidget;

	double	_focallength;
	double	_pixelsize;
	int	_angle;
	double	_guiderate;
	int	_declination;
	bool	_rainvert;
	bool	_decinvert;

	snowstar::Calibration	_cal;

public:
	explicit calibrationcalculatordialog(snowstar::GuiderPrx guider,
		snowstar::GuiderFactoryPrx guiderfactory,
		snowstar::ControlType type,
		calibrationwidget *calwidget,
		QWidget *parent = 0);
	~calibrationcalculatordialog();

private:
	Ui::calibrationcalculatordialog *ui;

	void	updateCalibration();

public slots:
	void	angleChanged(int);
	void	declinationChanged(int);
	void	rainvertChanged(int);
	void	decinvertChanged(int);
	void	acceptCalibration();
	void	rejectCalibration();
};

} // namespace snowgui

#endif // CALIBRATIONCALCULATORDIALOG_H
