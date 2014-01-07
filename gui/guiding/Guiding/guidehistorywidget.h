/*
 * guidehistorywidget.h -- Widget to display a history of tracking errors
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _guidehistorywidget_h
#define _guidehistorywidget_h

#include <QWidget.h>
#include <deque>
#include <cmath>

/**
 * \brief Widget to dispay the tracking error history
 */
class GuideHistoryWidget : public QWidget {

	Q_OBJECT

	std::deque<double>	data;
	double	a, b;
	double	ycorr(double x) const { return a * x + b; }
	void	drawCurve();
	QColor	_color;
public:
	explicit GuideHistoryWidget(QWidget *parent = 0);
	virtual ~GuideHistoryWidget();
	void	setColor(const QColor& color) { _color = color; }
	const QColor&	color() const { return _color; }

	void	paintEvent(QPaintEvent *event);

public slots:
	void	add(double value);
	void	add(const std::list<double>& values);

private slots:

};

#endif /* _guidehistorywidget_h */
