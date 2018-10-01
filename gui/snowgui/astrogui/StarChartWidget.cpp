/*
 * StarChartWidget.cpp -- 
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "StarChartWidget.h"
#include <AstroDebug.h>
#include <QPainter>
#include <QMouseEvent>

using namespace astro::catalog;

namespace snowgui {

/**
 * \brief Construct a new Star chart
 */
StarChartWidget::StarChartWidget(QWidget *parent) : QWidget(parent),
	_converter(astro::RaDec(), astro::Angle((M_PI / 180) / 100.),
		astro::Angle(0)) {
	_resolution.degrees(1 / 100.); // 1 deg/100 pixels
	_limit_magnitude = 10;
	_negative = false;
	_show_grid = true;
}

/**
 * \brief Destroy the star chart
 */
StarChartWidget::~StarChartWidget() {
}

/**
 * \brief Redraw the star chart
 */
void	StarChartWidget::paintEvent(QPaintEvent * /* event */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "redraw the star chart");
	draw();
}

QPointF	StarChartWidget::convert(const astro::RaDec& radec) {
	astro::Point	p = _converter(radec);
	QPointF	P(_center.x() + p.x(), _center.y() - p.y());
	return P;
}

/**
 * \brief Draw a star
 */
void	StarChartWidget::drawStar(QPainter& painter, const Star& star) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "draw star %s", star.toString().c_str());
	// convert the position into a x/y coordinates
	astro::Point	p = _converter(star.position(2000));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Point: %s", p.toString().c_str());

	// if the point is outside the widget rectangle, we quit
	double	w = 5 + width() / 2.;
	double	h = 5 + height() / 2.;
	if ((fabs(p.x()) > w) || (fabs(p.y()) > h)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "star outside rectangle");
		return;
	}

	// compute the center
	QPointF	starcenter(_center.x() + p.x(), _center.y() - p.y());

	// determine the radius
	float	sr = 5 - star.mag() / 2;
	if (sr < 0.8) {
		sr = 0.8;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"drawing star %s at position (%.1f, %.1f), r=%.1f",
		star.toString().c_str(), p.x(), p.y(), sr);

	// draw the circle
	QPainterPath	starcircle;
	starcircle.addEllipse(starcenter, sr, sr);

	// draw the circle
	QColor	black(0,0,0);
	QColor	white(255,255,255);
	painter.fillPath(starcircle, (_negative) ? black : white);
}

/**
 * \brief Draw a Line segment
 */
void	StarChartWidget::drawLine(QPainter& painter, const astro::RaDec& from,
		const astro::RaDec& to) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Grid line from %s to %s",
		from.toString().c_str(), to.toString().c_str());
	QPointF	From = convert(from);
	QPointF	To = convert(to);
	painter.drawLine(From, To);
}

/**
 * \brief Draw the coordinate grid
 */
void	StarChartWidget::drawGrid(QPainter& painter) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "draw the coordinate grid, center %s",
		_direction.toString().c_str());

	// prepare the pen used for drawing 
	QPen	pen;
	QColor	blue(102,204,255);
	QColor	darkblue(51,0,255);
	pen.setColor((_negative) ? darkblue : blue);
	pen.setWidth(1);
	painter.setPen(pen);

	// start drawing the grid lines spaced 1degree or 4minutes
	int	ralines = 2 + width() / 100;
	int	declines = 2 + height() / 100;
	astro::Angle	initialra;
	initialra.degrees(trunc(_direction.ra().degrees()));
	astro::Angle	rastep(M_PI / 180);
	initialra = initialra - ralines * rastep;
	astro::Angle	initialdec;
	initialdec.degrees(trunc(_direction.dec().degrees()));
	astro::Angle	decstep(M_PI / 180);
	initialdec = initialdec - declines * decstep;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "RA lines = %d, DEC lines = %d",
		ralines, declines);
	
	// line parameters
	astro::Angle	ra;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "RA line spacing %s",
		rastep.hms().c_str());
	astro::Angle	dec;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "DEC line spacing %s",
		decstep.dms().c_str());

	// draw RA lines
	for (int r = 0; r <= 2 * ralines; r++) {
		ra = initialra + r * rastep;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "RA = %s", ra.hms().c_str());
		astro::Angle	dstep = 0.1 * decstep;
		for (int d = 0; d <= 20 * declines; d++) {
			dec = initialdec + d * dstep;
			astro::RaDec	from(ra, dec);
			astro::RaDec	to(ra, dec + dstep);
			drawLine(painter, from, to);
		}
	}

	// draw DEC lines
	for (int d = 0; d <= 2 * declines; d++) {
		dec = initialdec + d * decstep;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "DEC = %s", dec.dms().c_str());
		astro::Angle	rstep = 0.1 * rastep;
		for (int r = 0; r <= 20 * ralines; r++) {
			ra = initialra + r * rstep;
			astro::RaDec	from(ra, dec);
			astro::RaDec	to(ra + rstep, dec);
			drawLine(painter, from, to);
		}
	}
}

/**
 * \brief Redraw the star chart
 */
void	StarChartWidget::draw() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "draw()");
	QPainter	painter(this);
	painter.setRenderHint(QPainter::Antialiasing);

	// find the center, as the size may have changed
	_center = QPointF(width() / 2., height() / 2.);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "_center = (%.1f,%.1f)",
		_center.x(), _center.y());

	// fill the entire rectangle black/white
	QPainterPath	rectangle;
	rectangle.moveTo(0, 0);
	rectangle.lineTo(width(), 0);
	rectangle.lineTo(width(), height());
	rectangle.lineTo(0, height());
	rectangle.lineTo(0, 0);

	QColor	white(255,255,255);
	QColor	black(0,0,0);
	painter.fillPath(rectangle, (_negative) ? white : black);

	// draw the grid
	if (show_grid()) {
		drawGrid(painter);
	}

	// draw the stars
	if (_stars) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%d stars to draw",
			_stars->size());
		Catalog::starset::const_iterator        i;
		for (i = _stars->begin(); i != _stars->end(); i++) {
			drawStar(painter, *i);
		}
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no stars");
	}
}

/**
 * \brief Change the center 
 *
 * This triggers getting a new set of stars from the catalog
 */
void	StarChartWidget::directionChanged(astro::RaDec direction) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "change direction to %s",
			direction.toString().c_str());
	if (_direction == direction) {
		return;
	}
	_direction = direction;

	// update the converter
	_converter = astro::ImageCoordinates(_direction, _resolution,
			astro::Angle(0));

	// compute the width and height of the star chart
	astro::Angle	rawidth(1.5 * width() * _resolution.radians());
	astro::Angle	decheight(1.5 * height() * _resolution.radians());
	SkyWindow	window(_direction, rawidth, decheight);

	// get the stars from the catalog
	// XXX this takes too much time, we should do this in a separate
	// XXX thread
	CatalogPtr catalog = CatalogFactory::get();
	MagnitudeRange	magrange(-30, limit_magnitude());
	_stars = catalog->find(window, magrange);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d stars found", _stars->size());

	// let the repaint event handle the redrawing
	repaint();
}

/**
 * \brief Common method for mouse events
 */
void	StarChartWidget::mouseCommon(QMouseEvent *event) {
	// get the pixel coordinates from the event relative to the center
	astro::Point	offset(event->pos().x() - _center.x(),
				_center.y() - event->pos().y());

	// convert into RA/DEC
	astro::RaDec	radec = _converter(offset);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "RA/DEC of point: %s",
		radec.toString().c_str());
	emit pointSelected(radec);
}

/**
 * \brief Handle mouse click
 */
void	StarChartWidget::mousePressEvent(QMouseEvent *event) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "handle mouse click at (%d,%d)",
		event->pos().x(), event->pos().y());
	mouseCommon(event);
}

/**
 * \brief Handle mouse move
 */
void	StarChartWidget::mouseMoveEvent(QMouseEvent *event) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "handle mouse move to (%d,%d)",
		event->pos().x(), event->pos().y());
	mouseCommon(event);
}

} // namespace snowgui
