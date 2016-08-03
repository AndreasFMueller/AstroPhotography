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
	bool	_byposition;

private:
	Scaler	scaler;

public:
	explicit FocusPointsWidget(QWidget *parent = NULL);
	~FocusPointsWidget();

	void	add(astro::image::ImagePtr, unsigned short position);
	bool	byposition() const { return _byposition; }

	void	paintEvent(QPaintEvent *event);

protected:
	int	showPositionAsTooltip(QMouseEvent *);
	void	mousePressEvent(QMouseEvent *);
	void	mouseMoveEvent(QMouseEvent *);

signals:
	void	positionSelected(int);

private:
	void	drawPoints(QPainter& painter, const Scaler::pointlist& pl);
	void	drawBySequence(QPainter& painter);
	void	drawByPosition(QPainter& painter);
	void	draw();

public slots:
	void	clear();
	void	setByPosition(bool b);
};

} // namespace snowgui

#endif /* _FocusPointsWidget_h */
