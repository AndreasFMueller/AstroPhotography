/*
 * calibrationwidget.h
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef CALIBRATIONWIDGET_H
#define CALIBRATIONWIDGET_H

#include <QWidget>
#include <guider.h>
#include <QTimer>
#include <AstroCoordinates.h>

namespace snowgui {

namespace Ui {
	class calibrationwidget;
}

class guidercontrollerwidget;

/**
 * \brief Widget to display and select a calibration for a guider
 */
class calibrationwidget : public QWidget {
	Q_OBJECT

	snowstar::ControlType		_controltype;
	std::string			_instrumentname;
	snowstar::GuiderPrx		_guider;
	snowstar::GuiderFactoryPrx	_guiderfactory;

	snowstar::Calibration		_calibration;

	guidercontrollerwidget		*_guidercontroller;

	QTimer	_statusTimer;
	snowstar::GuiderState	_state;
public:
	void	setGuider(snowstar::ControlType controltype,
			const std::string& instrumentname,
			snowstar::GuiderPrx guider,
			snowstar::GuiderFactoryPrx guiderfactory,
			guidercontrollerwidget *guidercontroller);
	void	displayCalibration();
	void	setupState();
public:
	explicit calibrationwidget(QWidget *parent = 0);
	~calibrationwidget();

private:
	Ui::calibrationwidget *ui;

public slots:
	void	databaseClicked();
	void	calibrateClicked();
	void	calculateClicked();
	void	detailClicked();
	void	setCalibration(snowstar::Calibration);
	void	statusUpdate();

	void	setTelescope(astro::RaDec);
	void	setOrientation(bool);

private:
	astro::RaDec	_radec;
	bool		_west;

signals:
	void	telescopeChanged(astro::RaDec);
	void	orientationChanged(bool);
	void	calibrationChanged();
};

} // namespace snowgui

#endif // CALIBRATIONWIDGET_H
