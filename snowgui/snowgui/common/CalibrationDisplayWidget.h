/*
 * calibrationdisplaywidget.h -- widget to display a calibration
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _CalibrationDisplayWidget_h
#define _CalibrationDisplayWidget_h

#include <QWidget>
#include <guider.h>

namespace snowgui {

class CalibrationDisplayWidget : public QWidget {
	Q_OBJECT

	snowstar::Calibration	_calibration;
public:
	explicit CalibrationDisplayWidget(QWidget *parent = 0);
	virtual ~CalibrationDisplayWidget();

protected:
	void    paintEvent(QPaintEvent *);

private:
	void	draw();

public slots:
	void	setCalibration(snowstar::Calibration);
};

} // namespace snowgui

#endif /* _CalibrationDisplayWidget_h */
