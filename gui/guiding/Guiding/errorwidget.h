/*
 * errorwidget.h -- Error display widget 
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _errorwidget_h
#define _errorwidget_h

#include <QWidget>
#include <guider.hh>

class ErrorWidget : public QWidget {
	Q_OBJECT

public:
	typedef std::pair<double, double>	point_t;
	typedef std::list<point_t>	points_t;
private:
	points_t	points;
	void	draw();
public:
	QString	label;
	QColor	color;
	void	addPoint(const point_t& point);
	void	clear();
	void	paintEvent(QPaintEvent *event);
public:
	explicit	ErrorWidget(QWidget *parent = 0);
	virtual	~ErrorWidget();
public slots:
private slots:
};

#endif /* _errorwidget_h */
