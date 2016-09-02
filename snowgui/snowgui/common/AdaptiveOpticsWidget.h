/*
 * AdaptiveOpticsWidget.h -- A widget to show and change the current AO position
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AdaptiveOpticsWidget_h
#define _AdaptiveOpticsWidget_h

#include <QLabel>

namespace snowgui {

class AdaptiveOpticsWidget : public QLabel {
	Q_OBJECT
	double	_x, _y;
	double	_radius;
	void	draw();
public:
	explicit AdaptiveOpticsWidget(QWidget *parent = NULL);
	virtual ~AdaptiveOpticsWidget();
public slots:
	bool	setPoint(QPointF);
	void	paintEvent(QPaintEvent *event);
	void	emitPoint(QPoint p);
signals:
	void	pointSelected(QPointF);
protected:
	void mousePressEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
};

} // namespace snowgui

#endif /* _AdaptiveOpticsWidget_h */
