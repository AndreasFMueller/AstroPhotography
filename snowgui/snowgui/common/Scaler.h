/*
 * Scaler.h -- scaling transformation for images
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Scaler_h
#define _Scaler_h

#include <QPoint>
#include <list>
#include <vector>
#include <FocusPoints.h>

namespace snowgui {

class Scaler {
	double	_minx, _maxx, _miny, _maxy;
	double	_width, _height;
	double	_scalex, _scaley;
	double	_bottommargin;
	void	setup(double bottommargin);
public:
	typedef QPoint	point;
	typedef std::list<QPoint>	pointlist;
	Scaler(double width = 1, double height = 1, double bottommargin = 0);
	Scaler(double minx, double maxx, double miny, double maxy,
		double width, double height, double bottommargin);
	double	x(double _x) const;
	double	y(double _y) const;
	QPoint	operator()(double _x, double _y) const;
	QPoint	operator()(const QPoint& p) const;
	QPoint	inverse(const QPoint& p) const;
	pointlist	list(const std::vector<FocusRawPoint>& fpv) const;
	double	bottommargin() const;
	void	bottommargin(double b);
	std::string	toString() const;
};

} // namespace snowgui

#endif /* _Scaler_h */
