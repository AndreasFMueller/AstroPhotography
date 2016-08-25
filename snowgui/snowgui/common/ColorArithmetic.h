/*
 * ColorArithmetic.h -- display stddev as a colored rectangles
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _ColorArithmetic_h
#define _ColorArithmetic_h

#include <QColor>

namespace snowgui {

class Color {
	double	_r;
	double	_g;
	double	_b;
public:
	Color(double r, double g, double b);
	Color();
	Color(const QColor&);
	Color	operator+(const Color& other) const;
	Color	operator*(const double) const;
	Color	operator-() const;
	QColor	qcolor() const;
};

} // namespace snowgui

#endif /* _ColorArithmetic_h */
