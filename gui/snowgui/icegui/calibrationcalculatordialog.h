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
#include <AstroCoordinates.h>

namespace snowgui {

namespace Ui {
	class calibrationcalculatordialog;
}

class calibrationcalculatordialog : public QDialog {
	Q_OBJECT

	snowstar::GuiderPrx		_guider;
	snowstar::GuiderFactoryPrx	_guiderfactory;

	double	_focallength;	// focal length in [m]
	double	_pixelsize;	// pixel size in [m]
	double	_angle;		// rotation angle in [degrees]
	double	_guiderate;	// guider rate, default 0.5
	double	_declination;	// declination in [degrees]
	double	_decrate;	// rate of the declination drive
				// relative to the RA drive
	bool	_telescopewest;	// wether or not the telescope was on the west
	bool	_decinvert;	// do the optics vertically flip the image?

	snowstar::Calibration	_cal;

public:
	explicit calibrationcalculatordialog(snowstar::GuiderPrx guider,
		snowstar::GuiderFactoryPrx guiderfactory,
		snowstar::ControlType type,
		calibrationwidget *calwidget,
		QWidget *parent = 0);
	~calibrationcalculatordialog();

	double	focallength() const { return _focallength; }
	double	pixelsize() const { return _pixelsize; }
	double	angle() const { return _angle; }
	double	guiderate() const { return _guiderate; }
	double	decrate() const { return _decrate; }
	double	declination() const { return _declination; }
	bool	telescopewest() const { return _telescopewest; }
	bool	decinvert() const { return _decinvert; }

#if 0
	void	focallength(double f);
	void	pixelsize(double s);
	void	angle(double a);
	void	guiderate(double a);
	void	decrate(double a);
	void	declination(double d);
	void	telescopewest(bool w);
	void	decinvert(bool i);
#endif

private:
	Ui::calibrationcalculatordialog *ui;

	void	updateCalibration();

signals:
	void	newCalibration(snowstar::Calibration);

public slots:
	void	angleChanged(double);
	void	declinationChanged(double);
	void	decrateChanged(double);

	void	decinvertChanged(int);

	void	acceptCalibration();
	void	rejectCalibration();

	void	setTelescope(astro::RaDec radec);
	void	setOrientation(bool);
	void	orientationChanged(int);
};

} // namespace snowgui

#endif // CALIBRATIONCALCULATORDIALOG_H
