/* 
 * SkyDisplayWidget.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "SkyDisplayWidget.h"
#include <AstroFormat.h>
#include <algorithm>

using namespace astro::catalog;

namespace snowgui {

static std::string	S(const astro::AzmAlt& a) {
	return astro::stringprintf("azm=%.2f,alt=%.2f", a.azm().degrees(),
		a.alt().degrees());
}

static bool	visible(const astro::AzmAlt& a) {
	return (a.alt().radians() > 0);
}

/**
 * \brief Construct the SkyDisplay
 *
 * \param parent	parent widget
 */
SkyDisplayWidget::SkyDisplayWidget(QWidget *parent) : QWidget(parent) {
	// get all the stars from the BSC catalog
	CatalogPtr catalog = CatalogFactory::get();
	SkyWindow	windowall;
	MagnitudeRange	magrange(-30, 6);
	_stars = catalog->find(windowall, magrange);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d stars", _stars->size());
	_converter = NULL;

	// for the time being, wie fake the logitude an latitude
	_position.longitude().degrees(8.83);
	_position.latitude().degrees(47.15);
}

/**
 * \brief Destroy the SkyDisplay
 */
SkyDisplayWidget::~SkyDisplayWidget() {
	if (_converter) {
		delete _converter;
	}
}

/**
 * \brief convert the celestial coordinates to altitude and azimut
 *
 * \param radec		the celestial coordinates to convert
 */
astro::AzmAlt	SkyDisplayWidget::convert(const astro::RaDec& radec) {
	if (NULL == _converter) {
		_converter = new astro::AzmAltConverter(_position);
	}
	return (*_converter)(radec);
}

/**
 * \brief convert Azimuth and Altitude to pixel coordinates
 *
 * \param azmalt	azimuth and altitude to convert
 */
QPoint	SkyDisplayWidget::convert(const astro::AzmAlt& azmalt) {
	float	r = _radius * (1 - azmalt.alt().radians() / (M_PI / 2));
	double	phi = azmalt.azm().radians();
	QPoint	starcenter(_center.x() + r * cos(phi),
			_center.y() - r * sin(phi));
	return starcenter;
}

/**
 * \brief Draw a star
 *
 * \param painter	the QPainter to use to draw the star
 * \param star		the star to draw
 */
void	SkyDisplayWidget::drawStar(QPainter& painter, const Star& star) {
	// find azimuth and altitude
	astro::AzmAlt	azmalt = convert(star.position(2000));

	// decide whether to draw the star at all
	if (!visible(azmalt)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "skipping star %s",
			star.toString().c_str());
		return;
	}

	// compute coordinates where to draw the star
	QPoint	starcenter = convert(azmalt);

	// compute the radius of the circle from the magnitude of the star
	float	sr = 4 - star.mag() / 1.8;
	if (sr < 0.8) {
		sr = 0.8;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "drawing star %s at position %s r=%.1f",
		star.toString().c_str(), S(azmalt).c_str(), sr);

	// now prepare a path for the star
	QPainterPath	starcircle;
	starcircle.addEllipse(starcenter, sr, sr);

	// draw the star
	QColor	white(255, 255, 255);
	painter.fillPath(starcircle, white);
}

/**
 * \brief Draw a telescope marker
 *
 * \param painter	the QPainter to use to draw the telescope marker
 */
void	SkyDisplayWidget::drawTelescope(QPainter& painter) {
	// find out where to draw the marker
	astro::AzmAlt	azmalt = convert(telescope());
	if (!visible(azmalt)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "telescope below horizon");
		return;
	}

	// set up drawing the telescope marker
	QPainterPath	telescopemarker;
	QPen	pen(Qt::SolidLine);
	pen.setWidth(2);
	QColor	red(255, 0, 0);
	pen.setColor(red);
	painter.setPen(pen);

	// compose the path
	QPoint	markerpoint = convert(azmalt);
	telescopemarker.addEllipse(markerpoint, 7, 7);

	// draw the marker in red
	painter.drawPath(telescopemarker);
}

/**
 * \brief paint the sky anew
 */
void	SkyDisplayWidget::draw() {
	// set up the parameters of drawing: radius and center
	_radius = std::min<float>(width() / 2., height() / 2);
	_center = QPoint(width() / 2, height() / 2);

	// set up a painter for drawing operations
	QPainter	painter(this);
	painter.setRenderHint(QPainter::Antialiasing);

	// draw a black circle
	QPainterPath	circle;
	circle.addEllipse(_center, _radius, _radius);
	QColor	black(0, 0, 0);
	painter.fillPath(circle, black);

	// draw the stars
	Catalog::starset::const_iterator	i;
	for (i = _stars->begin(); i != _stars->end(); i++) {
		drawStar(painter, *i);
	}

	// draw the telescope marker
	drawTelescope(painter);
}

/**
 * \brief  Paint event handler
 */
void	SkyDisplayWidget::paintEvent(QPaintEvent * /* event */) {
	draw();
}

/**
 * \brief Redraw the sky with a new postion of the telescope marker
 */
void	SkyDisplayWidget::telescopeChanged(astro::RaDec radec) {
	telescope(radec);
	repaint();
}

} // namespace snowgui


