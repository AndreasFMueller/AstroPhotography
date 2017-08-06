/*
 * FocusPointsWidget.h -- widget to graphically display focus points
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _FocusPointsWidget_h
#define _FocusPointsWidget_h

#include <QWidget>
#include <FocusPoints.h>
#include <Scaler.h>

namespace snowgui {

class FocusPointsWidget : public QWidget {
	Q_OBJECT

protected:
	FocusPoints	_focuspoints;
	FocusPointOrder::order_t	_order;
	FocusPointMeasure::measure_t	_measure;

private:
	Scaler	scaler;

public:
	explicit FocusPointsWidget(QWidget *parent = NULL);
	~FocusPointsWidget();

	void	add(astro::image::ImagePtr, long position);

	void	paintEvent(QPaintEvent *event);
	void	setOrder(FocusPointOrder::order_t);
	void	setMeasure(FocusPointMeasure::measure_t);

protected:
	int	showPositionAsTooltip(QMouseEvent *);
	void	mousePressEvent(QMouseEvent *);
	void	mouseMoveEvent(QMouseEvent *);

signals:
	void	positionSelected(int);

private:
	void	drawPoints(QPainter& painter, const Scaler::pointlist& pl);
	void	draw();

public slots:
	void	clear();
};

} // namespace snowgui

#endif /* _FocusPointsWidget_h */
