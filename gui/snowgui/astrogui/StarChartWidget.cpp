/*
 * StarChartWidget.cpp -- 
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "StarChartWidget.h"
#include <AstroDebug.h>
#include <QPainter>

using namespace astro::catalog;

namespace snowgui {

/**
 * \brief Construct a new Star chart
 */
StarChartWidget::StarChartWidget(QWidget *parent) : QWidget(parent),
	_converter(astro::RaDec(), astro::Angle(1 / 200.), astro::Angle(0)) {
	_resolution.degrees(1 / 200.); // 1 deg/200 pixels
	_limit_magnitude = 10;
	_negative = false;
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

} // namespace snowgui
