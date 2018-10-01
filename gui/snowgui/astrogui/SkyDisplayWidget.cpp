/* 
 * SkyDisplayWidget.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "SkyDisplayWidget.h"
#include <AstroFormat.h>
#include <algorithm>
#include <QMouseEvent>

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

	// show the altaz grid by default
	_show_altaz = true;
	_show_radec = true;
	_show_constellations = true;
	_show_labels = true;
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
QPointF	SkyDisplayWidget::convert(const astro::AzmAlt& azmalt) {
	float	r = _radius * (1 - azmalt.alt().radians() / (M_PI / 2));
	double	phi = azmalt.azm().radians();
	QPointF	starcenter(_center.x() + r * sin(phi),
			_center.y() + r * cos(phi));
	return starcenter;
}

/**
 * \brief Draw a line
 *
 * This method takes care that lines that have both alts negative are not
 * drawn at all, and lines that have precisely one alt negative are 
 * interpolated in a way so that they can be drawn precisely to the boundary.
 *
 * \param painter	painter to use for drawing
 * \param from		initial point
 * \param to		target point
 */
void	SkyDisplayWidget::drawLine(QPainter& painter, const astro::RaDec& from,
		const astro::RaDec& to) {
	astro::AzmAlt	From = convert(from);
	astro::AzmAlt	To = convert(to);

	// segment completely outside the circle
	if ((From.alt().radians() < 0) && (To.alt().radians() < 0)) {
		return;
	}

	// segment completely inside the circle
	if ((From.alt().radians() > 0) && (To.alt().radians() > 0)) {
		QPointF	F = convert(From);
		QPointF	T = convert(To);
		painter.drawLine(F, T);
		return;
	}

	// remaining cases, start with the differences
	debug(LOG_DEBUG, DEBUG_LOG, 0, "divide %.3f/%.3f - %.3f/%.3f",
		From.azm().degrees(), From.alt().degrees(),
		To.azm().degrees(), From.alt().degrees());
	astro::Angle	delta = To.azm() - From.azm();
	if (delta.radians() > M_PI) {
		delta.radians(delta.radians() - 2 * M_PI);
	}
	if (delta.radians() < -M_PI) {
		delta.radians(delta.radians() + 2 * M_PI);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "divide delta: %.4f degrees",
		delta.degrees());

	// now divide the segment to get the boundary point
	double	t = From.alt().radians() / (To.alt() - From.alt()).radians();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "divide with t = %.3f", t);
	astro::AzmAlt	boundary;
	boundary.azm() = From.azm() - delta * t;

	// now draw the segment to the boundary
	QPointF	B = convert(boundary);
	if (From.alt().radians() > 0) {
		QPointF	F = convert(From);
		painter.drawLine(F, B);
	} else {
		QPointF	T = convert(To);
		painter.drawLine(B, T);
	}
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
	QPointF	starcenter = convert(azmalt);

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
	QPointF	markerpoint = convert(azmalt);
	telescopemarker.addEllipse(markerpoint, 7, 7);

	// draw the marker in red
	painter.drawPath(telescopemarker);
}

/**
 * \brief Draw a target marker
 *
 * \param painter	the QPainter to use to draw the telescope marker
 */
void	SkyDisplayWidget::drawTarget(QPainter& painter) {
	// find out where to draw the marker
	astro::AzmAlt	t = convert(_target);
	if (!visible(t)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "target below horizon");
		return;
	}

	// set up drawing the telescope marker
	QPainterPath	targetmarker;
	QPen	pen(Qt::SolidLine);
	pen.setWidth(2);
	QColor	green(0, 255, 0);
	pen.setColor(green);
	painter.setPen(pen);

	// compose the path
	QPointF	markerpoint = convert(t);
	targetmarker.addEllipse(markerpoint, 7, 7);

	// draw the marker in red
	painter.drawPath(targetmarker);
}

/**
 * \brief Draw the AltAz grid
 */
void	SkyDisplayWidget::drawAltaz(QPainter& painter) {
	// prepare a pen for drawing
	QPen	pen(Qt::SolidLine);
	pen.setWidth(1);
	QColor	orange(255,204,0);
	pen.setColor(orange);
	painter.setPen(pen);

	// draw the circles
	for (double r = 1./3; r < 1.1; r += 1./3) {
		QPainterPath	path;
		path.addEllipse(_center, r * _radius, r * _radius);
		painter.drawPath(path);
	}
	
	// draw the radial lines
	for (double a = 0; a < M_PI; a += M_PI / 6) {
		QPointF	p1(_center.x() + _radius * cos(a),
				_center.y() + _radius * sin(a));
		QPointF	p2(_center.x() - _radius * cos(a),
				_center.y() - _radius * sin(a));
		painter.drawLine(p1, p2);
	}
}

/**
 * \brief Draw the RA/DEC grid
 */
void	SkyDisplayWidget::drawRadec(QPainter& painter) {
	// prepare a pen for drawing
	QPen	pen(Qt::SolidLine);
	pen.setWidth(1);
	QColor	blue(102,204,255);
	pen.setColor(blue);
	painter.setPen(pen);

	// draw constant RA lines
	double	l = M_PI - 0.0001;
	double	decstep = l / 100;
	double	declimit = l/2 - decstep/2;
	for (double ra = 0; ra < 2 * M_PI; ra += M_PI / 6) {
		for (double dec = -l / 2; dec < declimit; dec += decstep) {
			astro::RaDec	from(ra, dec), to(ra, dec + decstep);
			drawLine(painter, from, to);
		}
	}

	// draw the DEC lines
	double	rastep = M_PI / 100;
	double	ralimit = 2 * M_PI - rastep/2;
	for (double dec = M_PI / 2; dec > -M_PI/2; dec -= M_PI / 6) {
		for (double ra = 0; ra < ralimit; ra += rastep) {
			astro::RaDec	from(ra, dec), to(ra + rastep, dec);
			drawLine(painter, from, to);
		}
	}
}

/**
 * \brief paint the sky anew
 */
void	SkyDisplayWidget::draw() {
	// set up the parameters of drawing: radius and center
	_radius = std::min<float>(width() / 2., height() / 2);
	_center = QPointF(width() / 2, height() / 2);

	// set up a painter for drawing operations
	QPainter	painter(this);
	painter.setRenderHint(QPainter::Antialiasing);

	// draw a black circle
	QPainterPath	circle;
	circle.addEllipse(_center, _radius, _radius);
	QColor	black(0, 0, 0);
	painter.fillPath(circle, black);

	// draw the grids
	if (show_altaz()) {
		drawAltaz(painter);
	}
	if (show_radec()) {
		drawRadec(painter);
	}
	if (show_constellations()) {
		drawConstellations(painter);
	}

	// draw the stars
	Catalog::starset::const_iterator	i;
	for (i = _stars->begin(); i != _stars->end(); i++) {
		drawStar(painter, *i);
	}

	// draw the telescope marker
	drawTelescope(painter);
	if (show_target()) {
		drawTarget(painter);
	}

	if (show_labels()) {
		drawLabels(painter);
	}
}

/**
 * \brief Draw direction labels
 */
void	SkyDisplayWidget::drawLabels(QPainter& painter) {
	QPen	pen;
	QColor	white(255,255,255);
	pen.setColor(white);
	painter.setPen(pen);

	painter.drawText(_center.x() - 10, _center.y() + _radius - 20,
		20, 20, Qt::AlignCenter, QString("S"));

	painter.drawText(_center.x() - 10, _center.y() - _radius,
		20, 20, Qt::AlignCenter, QString("N"));

	painter.drawText(_center.x() - _radius, _center.y() - 10,
		20, 20, Qt::AlignCenter, QString("E"));

	painter.drawText(_center.x() + _radius - 20, _center.y() - 10,
		20, 20, Qt::AlignCenter, QString("W"));

}

/**
 * \brief  Paint event handler
 */
void	SkyDisplayWidget::paintEvent(QPaintEvent * /* event */) {
	// make sure we have a update to date converter
	if (NULL != _converter) {
		delete _converter;
	}
	_converter = new astro::AzmAltConverter(_position);
	_converter->update(); // updates to the current time

	// now start drawing
	draw();
}

/**
 * \brief Redraw the sky with a new postion of the telescope marker
 *
 * \param radec		direction into which the telescope is pointing
 */
void	SkyDisplayWidget::telescopeChanged(astro::RaDec radec) {
	telescope(radec);
	repaint();
}

/**
 * \brief Redraw the sky with a new position of the telescope on earth
 *
 * \param longlat	geographical position of the observatory on earth
 */
void	SkyDisplayWidget::positionChanged(astro::LongLat longlat) {
	position(longlat);
	repaint();
}

/**
 * \brief All mouse events are processed the same way with this method
 *
 * \param e	the mouse event to process
 */
void	SkyDisplayWidget::mouseCommon(QMouseEvent *e) {
	double	deltax = e->pos().x() - _center.x();
	double	deltay = e->pos().y() - _center.y();

	// compute the radius
	double	f = hypot(deltax, deltay) / _radius;
	if (f > 1) {
		return;
	}

	// convert the radius to an angle
	astro::Angle	s((1 - f) * M_PI / 2);

	astro::AzmAlt	azmalt(astro::arctan2(deltax, deltay), s);
	
	debug(LOG_DEBUG, DEBUG_LOG, 0, "azm = %.3f, alt = %.3f",
		azmalt.azm().degrees(), azmalt.alt().degrees());

	// convert the to RaDec
	_target = _converter->inverse(azmalt);

	// emit the position
	emit pointSelected(_target);

	// draw a green circle to verify the position computed
	show_target(true);
	repaint();

}

/**
 * \brief Mouse press event
 *
 * computes the coordinates where the mouse was pressed and emits them
 * with the signal pointSelected(astro::RaDec)
 */
void	SkyDisplayWidget::mousePressEvent(QMouseEvent *e) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "mousePressEvent %d,%d",
		e->pos().x(), e->pos().y());
	mouseCommon(e);
}

/**
 * \brief Mouse move event
 *
 * computes the coordinates where the mouse was pressed and emits them
 * with the signal pointSelected(astro::RaDec)
 */
void	SkyDisplayWidget::mouseMoveEvent(QMouseEvent *e) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "mouseMoveEvent %d,%d",
		e->pos().x(), e->pos().y());
	mouseCommon(e);
}

/**
 * /brief Ensure that the object is deleted when it is hit by a close event
 */
void	SkyDisplayWidget::closeEvent(QCloseEvent * /* event */) {
	deleteLater();
}

} // namespace snowgui


