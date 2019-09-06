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
	bool	_pointlabels;

	double	_scale;
	QPointF	_center;
	QPointF	point(float xpxl, float ypxl) const;
	QPointF	point(double xpxl, double ypxl) const;
	QPointF	point(const QPointF& ppxl) const;
	QPointF	point(const snowstar::Point& ppxl) const;
public:
	explicit CalibrationDisplayWidget(QWidget *parent = 0);
	virtual ~CalibrationDisplayWidget();

	void	pointlabels(bool b) { _pointlabels = b; }
	bool	pointlabels() const { return _pointlabels; }

protected:
	void    paintEvent(QPaintEvent *);

private:
	void	draw();
	void	drawDisabled(QPainter&);
	void	drawEnabled(QPainter&);
	void	drawCommon(QPainter&, bool drawvectors, bool dim);

public slots:
	void	setCalibration(snowstar::Calibration);

protected slots:
	virtual void	changeEvent(QEvent *event);
};

} // namespace snowgui

#endif /* _CalibrationDisplayWidget_h */
