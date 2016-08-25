/*
 * ColorRectangles.h -- display stddev as a colored rectangles
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _ColorRectangles_h
#define _ColorRectangles_h

#include <ColorArithmetic.h>
#include <QPainter>
#include <set>

namespace snowgui {

class ColorChange : public Color {
	double	_y;
public:
	ColorChange(double y, double r, double g, double b);
	ColorChange(double y, const Color& color);
	ColorChange(double y, const QColor& qcolor);
	bool	operator<(const ColorChange& other) const;
	double	y() const { return _y; }
};

class ColorRectangles : public std::set<ColorChange> {
public:
	void	draw(QPainter&, int w) const;
	void	addRange(double bottom, double top, const Color& c);
};

} // namespace snowgui

#endif /* _ColorRectangles_h */
