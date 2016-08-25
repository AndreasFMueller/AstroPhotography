/*
 * ColorRectangles.cpp
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "ColorRectangles.h"
#include <AstroDebug.h>

namespace snowgui {

//////////////////////////////////////////////////////////////////////
// ColorChange implementation
//////////////////////////////////////////////////////////////////////
ColorChange::ColorChange(double y, double r, double g, double b)
	: Color(r, g, b), _y(y) {
}

ColorChange::ColorChange(double y, const Color& c) : Color(c), _y(y) {
}

ColorChange::ColorChange(double y, const QColor& c) : Color(c), _y(y) {
}

bool	ColorChange::operator<(const ColorChange& other) const {
	return _y < other._y;
}

//////////////////////////////////////////////////////////////////////
// ColorRectangles implementation
//////////////////////////////////////////////////////////////////////

void	ColorRectangles::draw(QPainter& painter, int width) const {
	Color	nextcolor;
	const_iterator	changeptr = begin();
	double	bottom = changeptr->y();
	nextcolor = nextcolor + *changeptr;
	while (end() != ++changeptr) {
		double	top = changeptr->y();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "draw from %f to %f",
			bottom, top);
		painter.fillRect(0, bottom, width, top - bottom,
			nextcolor.qcolor());
		nextcolor = nextcolor + *changeptr;
		bottom = top;
	} 
}

void	ColorRectangles::addRange(double bottom, double top, const Color& color){
	insert(ColorChange(bottom, color));
	insert(ColorChange(top, -color));
}

} // namespace snowgui
