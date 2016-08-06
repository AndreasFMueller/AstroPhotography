/*
 * calibrationwidget.h
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef CALIBRATIONWIDGET_H
#define CALIBRATIONWIDGET_H

#include <QWidget>
#include <guider.h>

namespace Ui {
	class calibrationwidget;
}

namespace snowgui {

class guidercontrollerwidget;

/**
 * \brief Widget to display and select a calibration for a guider
 */
class calibrationwidget : public QWidget {
	Q_OBJECT

	snowstar::ControlType		_controltype;
	snowstar::GuiderDescriptor	_guiderdescriptor;
	snowstar::GuiderPrx		_guider;
	snowstar::GuiderFactoryPrx	_guiderfactory;

	snowstar::Calibration		_calibration;

	guidercontrollerwidget		*_guidercontroller;
public:
	void	setGuider(snowstar::ControlType controltype,
			snowstar::GuiderDescriptor guiderdescriptor,
			snowstar::GuiderPrx guider,
			snowstar::GuiderFactoryPrx guiderfactory,
			guidercontrollerwidget *guidercontroller);
	void	displayCalibration();
public:
	explicit calibrationwidget(QWidget *parent = 0);
	~calibrationwidget();

private:
	Ui::calibrationwidget *ui;

public slots:
	void	databaseClicked();
	void	calibrateClicked();
	void	setCalibration(snowstar::Calibration);
};

} // namespace snowgui

#endif // CALIBRATIONWIDGET_H
