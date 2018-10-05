/*
 * StarChartWidget.cpp -- 
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "StarChartWidget.h"
#include <AstroDebug.h>
#include <AstroDevice.h>
#include <QPainter>
#include <QMouseEvent>
#include <QToolTip>

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
	_retriever = NULL;
	_retrieval_necessary = true;
	_mouse_pressed = false;

	qRegisterMetaType<astro::catalog::Catalog::starsetptr>("astro::catalog::Catalog::starsetptr");

	setMouseTracking(true);
}

/**
 * \brief Destroy the star chart
 */
StarChartWidget::~StarChartWidget() {
}

/**
 * \brief Redraw the star chart
 *
 * \param event	
 */
void	StarChartWidget::paintEvent(QPaintEvent * /* event */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "redraw the star chart");
	draw();
}

/**
 * \brief convert RA/DEC into point coordinates on the star chart
 */
QPointF	StarChartWidget::convert(const astro::RaDec& radec) {
	astro::Point	p = _converter(radec);
	QPointF	P(_center.x() + p.x(), _center.y() - p.y());
	return P;
}

/**
 * \brief Draw a star
 */
void	StarChartWidget::drawStar(QPainter& painter, const Star& star) {
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "draw star %s", star.toString().c_str());
	// convert the position into a x/y coordinates
	astro::Point	p = _converter(star.position(2000));
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "Point: %s", p.toString().c_str());

	// if the point is outside the widget rectangle, we quit
	double	w = 5 + width() / 2.;
	double	h = 5 + height() / 2.;
	if ((fabs(p.x()) > w) || (fabs(p.y()) > h)) {
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "star outside rectangle");
		return;
	}

	// compute the center
	QPointF	starcenter(_center.x() + p.x(), _center.y() - p.y());

	// determine the radius
	float	sr = 5 - star.mag() / 2;
	if (sr < 0.8) {
		sr = 0.8;
	}
	//debug(LOG_DEBUG, DEBUG_LOG, 0,
	//	"drawing star %s at position (%.1f, %.1f), r=%.1f",
	//	star.toString().c_str(), p.x(), p.y(), sr);

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
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "Grid line from %s to %s",
	//	from.toString().c_str(), to.toString().c_str());
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

	// construct a Skywindow to know which lines we have to draw
	astro::catalog::SkyWindow	window = SkyWindow::hull(_direction, 
				width() * _resolution, height() * _resolution);

	// start drawing the grid lines spaced 1degree or 4minutes

	// first find out where to strt
	astro::Angle	initialra = window.leftra();
	initialra.degrees(trunc(initialra.degrees()));
	astro::Angle	initialdec = window.bottomdec();
	initialdec.degrees(trunc(initialdec.degrees()));

	// find the maximum
	double	raspan = (window.rightra() - window.leftra()).degrees();
	int	ralines = 360;
	if (raspan > 0) {
		ralines = trunc(raspan + 2);
	}
	double	decspan = (window.topdec() - window.bottomdec()).degrees();
	int	declines = trunc(decspan + 2);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "RA lines = %d, DEC lines = %d",
		ralines, declines);

	// step size
	astro::Angle	rastep(M_PI / 180);
	astro::Angle	decstep(M_PI / 180);

	if (_direction.dec() > astro::Angle(80 * M_PI / 180)) {
		initialra.degrees(20 * trunc(initialra.degrees() / 20));
		ralines = ralines / 20;
		rastep.degrees(20);
	} else if (_direction.dec() > astro::Angle(70 * M_PI / 180)) {
		initialra.degrees(10 * trunc(initialra.degrees() / 10));
		ralines = ralines / 10;
		rastep.degrees(10);
	} else if (_direction.dec() > astro::Angle(60 * M_PI / 180)) {
		initialra.degrees(5 * trunc(initialra.degrees() / 5));
		ralines = ralines / 5;
		rastep.degrees(5);
	}

	// line parameters
	astro::Angle	ra;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "RA line spacing %s",
		rastep.hms().c_str());
	astro::Angle	dec;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "DEC line spacing %s",
		decstep.dms().c_str());

	// draw RA lines
	for (int r = 0; r <= ralines; r++) {
		ra = initialra + r * rastep;
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "RA = %s", ra.hms().c_str());
		astro::Angle	dstep = 0.1 * decstep;
		for (int d = 0; d <= 10 * declines; d++) {
			dec = initialdec + d * dstep;
			astro::RaDec	from(ra, dec);
			astro::RaDec	to(ra, dec + dstep);
			drawLine(painter, from, to);
		}
	}

	// draw DEC lines
	for (int d = 0; d <= declines; d++) {
		dec = initialdec + d * decstep;
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "DEC = %s", dec.dms().c_str());
		astro::Angle	rstep = 0.1 * rastep;
		for (int r = 0; r <= 10 * ralines; r++) {
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
 * \brief Work needed to start a new retrieval
 */
void	StarChartWidget::startRetrieval() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "initiate new star retrieval");
	// compute the width and height of the star chart
	astro::Angle	rawidth(1.5 * width() * _resolution.radians());
	astro::Angle	decheight(1.5 * height() * _resolution.radians());
	SkyWindow	window = SkyWindow::hull(_direction, rawidth, decheight);

	// if no retriever is running, start one
	if (NULL == _retriever) {
		// get the stars from the catalog
		_retriever = new StarChartRetriever();
		_retriever->limit_magnitude(limit_magnitude());
		_retriever->window(window);
		connect(_retriever,
			SIGNAL(starsReady(astro::catalog::Catalog::starsetptr)),
			this,
			SLOT(useStars(astro::catalog::Catalog::starsetptr)));
		connect(_retriever,
			SIGNAL(finished()),
			this,
			SLOT(workerFinished()));
		_retriever->start();
		_retrieval_necessary = false;
	} else {
		// remember to restart star retrieval as soon as the present
		// retrieval is complete
		_retrieval_necessary = true;
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

	// a new retrieval should only be started in tracking mode.
	// In any other mound we expect the state to change again very
	// soon
	if (_state == astro::device::Mount::TRACKING) {
		// start the retrieval
		startRetrieval();

		// start the busy widget
		const int busysize = 100;
		_busywidget = new BusyWidget(this);
		_busywidget->resize(busysize, busysize);
		_busywidget->move(width()/2 - busysize/2,
			height()/2 - busysize/2);
		_busywidget->setVisible(true);
	}

	// let the repaint event handle the redrawing. Doing the repaing
	// always allows the image to track the movement of the telescope
	// which should make for a nice animation
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
 *
 * \param event		mouse event containing position information
 */
void	StarChartWidget::mousePressEvent(QMouseEvent *event) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "handle mouse click at (%d,%d)",
		event->pos().x(), event->pos().y());
	_mouse_pressed = true;
	mouseCommon(event);
}

void	StarChartWidget::mouseReleaseEvent(QMouseEvent * /* event */) {
	_mouse_pressed = false;
}

/**
 * \brief Handle mouse move
 *
 * \param event		 mouse event contining position information
 */
void	StarChartWidget::mouseMoveEvent(QMouseEvent *event) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "handle mouse move to (%d,%d)",
		event->pos().x(), event->pos().y());
	// if the button is pressed, we have to handle it like a click
	if (_mouse_pressed) {
		mouseCommon(event);
		return;
	}
	astro::Point	offset(event->pos().x() - _center.x(),
				_center.y() - event->pos().y());
	astro::RaDec	tiptarget = _converter(offset);
	QString	tiptext(astro::stringprintf("RA: %s DEC: %s",
			tiptarget.ra().hms(':', -1).c_str(),
			tiptarget.dec().dms(':', -1).c_str()).c_str());
	QToolTip::showText(event->globalPos(), tiptext);
}

/**
 * \brief receive a new set of stars from the worker thread
 *
 * This method receives a new set of stars from the worker thread and
 * redisplays the sky with this new set of stars
 *
 * \param stars		set of stars to display in the star chart
 */
void	StarChartWidget::useStars(astro::catalog::Catalog::starsetptr stars) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "receiveing %d new stars",
		stars->size());
	_stars = stars;
	repaint();
}

/**
 * \brief Handle termination of the thread
 */
void	StarChartWidget::workerFinished() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "StartChartRetriever has finished");
	// disconnect the retriever
	disconnect(_retriever,
		SIGNAL(starsReady(astro::catalog::Catalog::starsetptr)),
		0, 0);
	disconnect(_retriever, SIGNAL(finished()),
		0, 0);
	delete _retriever;
	_retriever = NULL;

	// remove the busy widget
	if (_busywidget) {
		delete _busywidget;
		_busywidget = NULL;
	}

	// start a new thread if necessary
	if (_retrieval_necessary) {
		startRetrieval();
	}
}

/**
 * \brief Update the state
 *
 * \param state		new state to remember
 */
void	StarChartWidget::stateChanged(astro::device::Mount::state_type state) {
	_state = state;
}

} // namespace snowgui
