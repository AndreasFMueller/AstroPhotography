/*
 * StarChartWidget.cpp -- 
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "StarChartWidget.h"
#include "SkyDisplayWidget.h"
#include <AstroDebug.h>
#include <AstroDevice.h>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QToolTip>
#include <QMenu>
#include <QAction>
#include <stdexcept>

using namespace astro::catalog;

namespace snowgui {

const astro::Angle	StarChartWidget::_standard_resolution(1 / 100.,
				astro::Angle::Degrees);
const astro::Angle	StarChartWidget::_wide_resolution(1 / 50.,
				astro::Angle::Degrees);

/**
 * \brief Construct a new Star chart
 *
 * \param parent	the parent QWidget
 */
StarChartWidget::StarChartWidget(QWidget *parent) : QWidget(parent),
	_converter(astro::RaDec(), astro::Angle((M_PI / 180) / 100.),
		astro::Angle(0)) {
	_resolution = _standard_resolution;
	_limit_magnitude = 10;
	_negative = false;
	_show_stars = true;
	_show_grid = true;
	_retriever = NULL;
	_retrieval_necessary = true;
	_mouse_pressed = false;
	_show_crosshairs = false;
	_show_directions = true;
	_show_cataloglabels = true;
	_show_planets = true;
	_show_sun = true;
	_show_moon = true;
	_show_constellations = true;
	_flip = true; // XXX is this correct?
	_show_deepsky = true;
	_show_tooltips = true;
	_state = astro::device::Mount::TRACKING; // safe assumption
	_legend = NULL;
	_gridstep_pixels = 128;
	_time = 0;

	qRegisterMetaType<astro::catalog::Catalog::starsetptr>("astro::catalog::Catalog::starsetptr");
	qRegisterMetaType<astro::catalog::DeepSkyObjectSetPtr>("astro::catalog::DeepSkyObjectSetPtr");
	qRegisterMetaType<astro::Angle>("astro::Angle");
	qRegisterMetaType<snowgui::ImagerRectangle>("snowgui::ImagerRectangle");

	setMouseTracking(true);

	// context menu
	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
		this, SLOT(showContextMenu(const QPoint &)));


	// launch a thread to retrieve the stars
	SkyStarThread	*skystarthread = new SkyStarThread(this);
	connect(skystarthread,
		SIGNAL(stars(astro::catalog::Catalog::starsetptr)),
		this,
		SLOT(useSky(astro::catalog::Catalog::starsetptr)));
	connect(skystarthread, SIGNAL(finished()),
		skystarthread, SLOT(deleteLater()));
	skystarthread->start();

	// launch a thred to retrieve the deep sky objects
	DeepSkyRetriever	*deepsky_thread
		= new DeepSkyRetriever(this);
	connect(deepsky_thread,
		SIGNAL(deepskyReady(astro::catalog::DeepSkyObjectSetPtr)),
		this,
		SLOT(useDeepSky(astro::catalog::DeepSkyObjectSetPtr)));
	//connect(deepsky_thread, SIGNAL(finished()),
	//	this, SLOT(workerFinished()));
	deepsky_thread->start();

	// create the outline catalog
	try {
		_outlines = astro::catalog::OutlineCatalogPtr(
			new astro::catalog::OutlineCatalog());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%d outlines", _outlines->size());
	} catch (std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "no outline catalog: %s",
			x.what());
	}

	// make sure the target follows the pointer
#if 0
	connect(this, SIGNAL(pointSelected(astro::RaDec)),
		this, SLOT(targetChanged(astro::RaDec)));
#endif
}

/**
 * \brief Destroy the star chart
 */
StarChartWidget::~StarChartWidget() {
}

void	StarChartWidget::gridstep_pixels_up() {
	if (_gridstep_pixels < 1024) {
		_gridstep_pixels <<= 1;
	}
}

void	StarChartWidget::gridstep_pixels_down() {
	if (_gridstep_pixels > 16) {
		_gridstep_pixels >>= 1;
	}
}

/**
 * \brief Redraw the star chart
 *
 * This event is called by the event loop when it is necessary to redraw
 * the widget.
 *
 * \param event		the QPaintEvent
 */
void	StarChartWidget::paintEvent(QPaintEvent * /* event */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "redraw the star chart");
	draw();
}

/**
 * \brief convert RA/DEC into point coordinates on the star chart
 *
 * The coordinates returned are flipped by 180 degrees if the _flip
 * flag is set
 *
 * \param radec		RA/DEC coordinates
 */
QPointF	StarChartWidget::convert(const astro::RaDec& radec) {
	astro::Point	p = _converter(radec);
	if (_flip) {
		p = -p;
	}
	QPointF	P(_center.x() + p.x(), _center.y() - p.y());
	return P;
}

/**
 * \brief Draw a star
 *
 * \param painter	the QPainter to draw on
 * \param star		the star object to display
 */
void	StarChartWidget::drawStar(QPainter& painter, const Star& star) {
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "draw star %s", star.toString().c_str());
	// convert the position into a x/y coordinates
	QPointF	p = convert(star.position(2000));
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "Point: %s", p.toString().c_str());

	// if the point is outside the widget rectangle, we quit
	if ((p.x() < -5) || (p.x() > (width() + 5))
		|| (p.y() < -5) || (p.y() > (height() + 5))) {
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "star outside rectangle");
		return;
	}

	// compute the center
	QPointF	starcenter(p);

	// determine the radius
	float	sr = 8 - star.mag() / 1.4;
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
 * \brief draw a deep sky object
 *
 * This draws a deep sky object at its position in the widget
 *
 * \param painter		the QPainter to draw on
 * \param deepskyobject		the deep sky object to display
 */
// XXX suggested improvements:
// XXX - background behind the object label to make it more readable
void	StarChartWidget::drawDeepSkyObject(QPainter& painter,
	const DeepSkyObject& deepskyobject) {

	// first find out whether we actually have to display the object
	if (_direction.scalarproduct(deepskyobject) < 0) {
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "ignore %s",
		//	deepskyobject.name.c_str());
		return;
	}

	// make sure we draw in red
	QPen	pen(Qt::SolidLine);
	switch (deepskyobject.classification) {
	case astro::catalog::DeepSkyObject::Galaxy:
	case astro::catalog::DeepSkyObject::MultipleSystem:
	case astro::catalog::DeepSkyObject::GalaxyInMultipleSystem:
		pen.setColor(Qt::red);
		break;
	case astro::catalog::DeepSkyObject::BrightNebula:
	case astro::catalog::DeepSkyObject::ClusterNebulosity:
		pen.setColor(Qt::green);
		break;
	case astro::catalog::DeepSkyObject::PlanetaryNebula:
		pen.setColor(Qt::magenta);
		break;
	case astro::catalog::DeepSkyObject::GlobularCluster:
		pen.setColor(Qt::yellow);
		break;
	case astro::catalog::DeepSkyObject::OpenCluster:
		pen.setColor(Qt::cyan);
		break;
	default:
		pen.setColor(Qt::gray);
		break;
	}
	pen.setWidth(2);
	painter.setPen(pen);

	// get the position
	QPointF	p = convert(deepskyobject.position(2000));

	// if the point is outside the widget rectangle, we quit
	if ((p.x() < -5) || (p.x() > (width() + 5))
		|| (p.y() < -5) || (p.y() > (height() + 5))) {
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "star outside rectangle");
		return;
	}

	//debug(LOG_DEBUG, DEBUG_LOG, 0, "draw deep sky object %s",
	//	deepskyobject.name.c_str());

	if ((_outlines) && (_outlines->has(deepskyobject.name))) {
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "%s has outline", 
		//	deepskyobject.name.c_str());
		QPainterPath	outlinepath;
		Outline	outline = _outlines->find(deepskyobject.name);
		QPointF	first = convert(*outline.begin());
		outlinepath.moveTo(first);
		auto i = outline.begin();
		i++;
		while (i != outline.end()) {
			QPointF	next = convert(*i);
			outlinepath.lineTo(next);
			i++;
		}
		outlinepath.closeSubpath();
		painter.drawPath(outlinepath);
	} else {
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "draw %s as circle",
		//	deepskyobject.name.c_str());
		// get the axes
		double	a = deepskyobject.axes().a1().radians()
				/ _resolution.radians();
		double	b = deepskyobject.axes().a2().radians()
				/ _resolution.radians();
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "axes: %f, %f", a, b);
		a /= 2;
		b /= 2;

		// draw an ellipse at the position of the object
		QPainterPath	ellipse;
		double	s = sin(deepskyobject.position_angle());
		double	c = cos(deepskyobject.position_angle());
		double	phi = 0;
		double	x = a * c;
		double	y = -a * s;
		ellipse.moveTo(p.x() + x, p.y() + y);
		double	phistep = M_PI / 50;
		while (phi < 2 * M_PI + phistep/2) {
			phi += phistep;
			x = a * cos(phi);
			y = b * sin(phi);
			ellipse.lineTo(p.x() + c * x + s * y,
				p.y() - s * x + c * y);
		}
		painter.drawPath(ellipse);
	}

	if (!show_cataloglabels()) {
		return;
	}

	// draw the name of the object
	painter.drawText(p.x() - 40, p.y() - 10, 80, 20,
		Qt::AlignCenter,
		QString(deepskyobject.name.c_str()));
	if (deepskyobject.name.size() == 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "unnamed object %s",
			deepskyobject.toString().c_str());
	}
}

/**
 * \brief Draw a Line segment
 *
 * \param painter	the QPainter to draw on
 * \param from		initial point on the celestial sphere
 * \param to		final point on the celestial sphere
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
 *
 * \param painter	the QPainter to draw on
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

	// use the gridcalculator to compute the grid parameters
	astro::utils::GridCalculator	gridcalculator(_direction,
					astro::Size(width(), height()),
					1 / _resolution.degrees());
	gridcalculator.gridsetup(gridstep_pixels());

	int	steps = 200;
	// draw RA grid lines
	for (int ra = gridcalculator.minra(); ra <= gridcalculator.maxra(); ra++) {
		astro::Angle	raangle = gridcalculator.ra(ra);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "drawing ra grid line for RA %s",
			raangle.hms().c_str());
		astro::TwoAngles	decrange = gridcalculator.angleRangeDEC(ra);
		astro::Angle	step = (decrange.a2() - decrange.a1()) / steps;
		for (int dec = 0; dec < steps; dec++) {
			astro::RaDec	from(raangle, decrange.a1() + dec * step);
			astro::RaDec	to(raangle, decrange.a1() + (dec + 1) * step);
			drawLine(painter, from, to);
		}
	}
	// draw DEC grid lines
	for (int dec = gridcalculator.mindec(); dec <= gridcalculator.maxdec(); dec++) {
		astro::Angle	decangle = gridcalculator.dec(dec);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "drawing dec grid line for DEC %s",
			decangle.dms().c_str());
		astro::TwoAngles	rarange = gridcalculator.angleRangeRA(dec);
		astro::Angle	step = (rarange.a2() - rarange.a1()) / steps;
		for (int ra = 0; ra < steps; ra++) {
			astro::RaDec	from(rarange.a1() + ra * step, decangle);
			astro::RaDec	to(rarange.a1() + (ra + 1) * step, decangle);
			drawLine(painter, from, to);
		}
	}
}

/**
 * \brief Draw the cross hairs
 *
 * Cross hairs are intended to help the user find the center of the chart
 *
 * \param painter	the QPainter to draw the crosshairs on
 */
void	StarChartWidget::drawCrosshairs(QPainter& painter) {
	// set the pen for drawing the cross hairs
	QPen	pen(Qt::SolidLine);
	pen.setColor(Qt::red);
	pen.setWidth(1);
	painter.setPen(pen);

	// compute the coordinates
	int	w = width();
	int	h = height();
	int	x = w / 2;
	int	y = h - h / 2;
	painter.drawLine(QPoint(0, y), QPoint(x - 5, y));
	painter.drawLine(QPoint(x + 5, y), QPoint(w - 1, y));
	painter.drawLine(QPoint(x, 0), QPoint(x, y - 5));
	painter.drawLine(QPoint(x, y + 5), QPoint(x, h - 1));
}

/**
 * \brief Draw the current target position
 *
 * Draw the current target position
 *
 * \param painter	the QPainter to draw the target on
 */
void	StarChartWidget::drawTarget(QPainter& painter) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "draw target marker");

	// set up drawing the target marker in green
	QPainterPath    targetmarker;
	QPen    pen(Qt::SolidLine);
	pen.setWidth(2);
	QColor  green(0, 255, 0);
	pen.setColor(green);
	painter.setPen(pen);

	// find out where to draw the marker
	QPointF	markerpoint = convert(_target);

	// compose the path
	targetmarker.addEllipse(markerpoint, 7, 7);

	// draw the marker in red
	painter.drawPath(targetmarker);

}

/**
 * \brief Method to draw the directions
 *
 * These are labels that indicate the directions north, south, east and west
 * on the chart. Diretions change then the image is flipped (like after a
 * meridian flip then the telescope is on the east side of the mount), where
 * we assume that the camera is oriented so that north is now down.
 *
 * \param painter	the QPainter to draw on
 */
void	StarChartWidget::drawDirections(QPainter& painter) {
	// prepare pen for color
	QPen	pen(Qt::SolidLine);
	pen.setColor(Qt::green);
	painter.setPen(pen);

	// draw the labels
	int	w = width();
	int	h = height();
	int	x = w / 2;
	int	y = h / 2;
	painter.drawText(0, y - 10, 20, 20, Qt::AlignCenter,
		(_flip) ? QString("W") : QString("E"));
	painter.drawText(w - 20, y - 10, 20, 20, Qt::AlignCenter,
		(_flip) ? QString("E") : QString("W"));
	painter.drawText(x - 10, 0, 20, 20, Qt::AlignCenter,
		(_flip) ? QString("S") : QString("N"));
	painter.drawText(x - 10, h - 20, 20, 20, Qt::AlignCenter,
		(_flip) ? QString("N") : QString("S"));
}

/**
 * \brief Method to draw the stars
 *
 * \param painter	painter to draw the stars on
 */
void	StarChartWidget::drawStars(QPainter& painter) {
	if (_stars) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%d stars to draw",
			_stars->size());
		for (auto i = _stars->begin(); i != _stars->end(); i++) {
			drawStar(painter, *i);
		}
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no stars");
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

	// draw the constellations
	if (show_constellations()) {
		drawConstellations(painter);
	}

	// draw the cross hairs
	if (show_crosshairs()) {
		drawCrosshairs(painter);
	}

	// draw the target
	if (show_target()) {
		drawTarget(painter);
	}

	// draw the direction labels
	if (show_directions()) {
		drawDirections(painter);
	}

	// draw the stars
	if (show_stars()) {
		drawStars(painter);
	}

	// draw the deep sky objects
	if (show_deepsky() && _deepsky) {
		DeepSkyObjectSet::const_iterator	i;
		for (i = _deepsky->begin(); i != _deepsky->end(); i++) {
			drawDeepSkyObject(painter, *i);
		}
	}

	// if the telescope is moving, we also display the sky stars
	if ((_state == astro::device::Mount::GOTO) && (_sky)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "adding sky stars");
		for (auto i = _sky->begin(); i != _sky->end(); i++) {
			drawStar(painter, *i);
		}
	}

	// draw the rectangles
	if (show_finder_rectangle()) {
		drawRectangle(painter, _finder_rectangle, _finder_resolution);
	}
	if (show_guider_rectangle()) {
		drawRectangle(painter, _guider_rectangle, _guider_resolution);
	}
	if (show_imager_rectangle()) {
		drawRectangle(painter, _imager_rectangle, _imager_resolution);
	}

	// draw the planets
	radius(10);
	if (show_planets()) {
		drawPlanets(painter, time());
	}
	radius(0.25 / _resolution.degrees());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "radius of sun/moon: %f", radius());
	if (show_moon()) {
		drawMoon(painter, time());
	}
	if (show_sun()) {
		drawSun(painter, time());
	}
}

/**
 * \brief Work needed to start a new retrieval
 *
 * This launches a StarCharRetriever thread on a given window. When the
 * retriever completes, it sends the selection from the star catalog
 * back to the star chart widget.
 *
 * If a retrieval thread is already running, the flag _retrieval_necessary
 * is set to remind us of the fact that we should run a new retrieval.
 */
void	StarChartWidget::startRetrieval() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "initiate new star retrieval, "
		"direction  = %s", _direction.toString().c_str());
	// compute the width and height of the star chart
	astro::Angle	rawidth(1.5 * width() * _resolution.radians());
	astro::Angle	decheight(1.5 * height() * _resolution.radians());
	SkyWindow	window = SkyWindow::hull(_direction, rawidth, decheight);

	// after the retrieval, the new chart center will be
	// the current telescope direction
	_chartcenter = _direction;

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
 * This triggers getting a new set of stars from the catalog. The selection
 * is such that all stars in contained in the image rectangle around 
 * the direction specified are included. Due to the compliated shape of
 * the selection area in RA/DEC coordinates, a somewhat larger area
 * called a SkyWindow is selected which inclues the image rectangle.
 * Additional stars don't really matter because they can be used to
 * immediately display a complete star chart after slight movements, which
 * improves responsiveness of the user interface.
 *
 * \param diretion	the direction w
 */
void	StarChartWidget::directionChanged(astro::RaDec direction) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "change direction to %s",
			direction.toString().c_str());
	if (_direction == direction) {
		return;
	}

	// update the converter to reflect the new center
	_converter = astro::ImageCoordinates(direction, _resolution,
			astro::Angle(0));

	// decide whether the change is big enough to warrant computing
	// a new catalog
	double  deltaRa = (_chartcenter.ra() - direction.ra()).radians();
        if (deltaRa > M_PI) { deltaRa -= 2 *M_PI; }
        if (deltaRa < -M_PI) { deltaRa += 2 * M_PI; }
        double  deltaDec = (_chartcenter.dec() - direction.dec()).radians();
        double  change = hypot(deltaRa, deltaDec);

	if (change < 0.01) {
		return;
	}

	// don't update the _direction member until now because otherwise
	// we will have no basis for the decision whether a new catalog
	// retrieval is warranted
	_direction = direction;

	// a new retrieval should only be started in tracking mode.
	// In any other mode we expect the state to change again very
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

	// let the repaint event handle the redrawing. Doing the repainting
	// always allows the image to track the movement of the telescope
	// which should make for a nice animation
	repaint();
}

/**
 * \brief Slot called when the target changes
 *
 * \param target	target marker position
 */
void	StarChartWidget::targetChanged(astro::RaDec target) {
	_target = target;
	repaint();
}

/**
 * \brief Common method for mouse events
 *
 * \param event		The mouse event to use for reading the coordinates.
 */
void	StarChartWidget::mouseCommon(QMouseEvent *event) {
	// get the pixel coordinates from the event relative to the center
	astro::Point	offset(event->pos().x() - _center.x(),
				_center.y() - event->pos().y());
	if (_flip) {
		offset = -offset;
	}

	// convert into RA/DEC
	astro::RaDec	radec = _converter(offset);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "RA/DEC of point: %s",
		radec.toString().c_str());
	if (_mouse_pressed) {
		emit pointSelected(radec);
	}
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

/**
 * \brief Handle events when the mouse is released
 */
void	StarChartWidget::mouseReleaseEvent(QMouseEvent * /* event */) {
	_mouse_pressed = false;
}

/**
 * \brief Handle mouse move
 *
 * Them ouse is tracked on the widget and a tooltip with the current
 * RA/DEC position is continually displayed. If the mouse is also
 * pressed, the position is also forwarded to the mouseCommon slot, which
 * emits the signal pointSelected.
 *
 * \param event		 mouse event contining position information
 */
void	StarChartWidget::mouseMoveEvent(QMouseEvent *event) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "handle mouse move to (%d,%d)",
		event->pos().x(), event->pos().y());
	// if the button is pressed, we have to handle it like a click
	if (_mouse_pressed) {
		mouseCommon(event);
	}

	if (!_show_tooltips) {
		return;
	}

	astro::Point	offset(event->pos().x() - _center.x(),
				_center.y() - event->pos().y());
	if (_flip) {
		offset = -offset;
	}
	astro::RaDec	tiptarget = _converter(offset);
	QString	tiptext(astro::stringprintf("RA: %s DEC: %s",
			tiptarget.ra().hms(':', -1).c_str(),
			tiptarget.dec().dms(':', -1).c_str()).c_str());
	QToolTip::showText(event->globalPos(), tiptext);
}

/**
 * \brief Receive a new set of stars from the worker thread
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
 * \brief Receive a full set of sky stars
 *
 * While moving the telescope, we cannot see the stars to high limiting
 * magnitude, but for the sake of an animation, we then use the sky stars
 * in addition to the more detailed stars of the field of view only.
 *
 * \param sky	set of stars representing the full sky
 */
void	StarChartWidget::useSky(astro::catalog::Catalog::starsetptr sky) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "receiving sky with %d stars",
		sky->size());
	_sky = sky;
	repaint();
}

/**
 * \brief Receive a set of deep sky objects
 *
 * \param deepsky	set of deep ky objects to display in the star chart
 */
void	StarChartWidget::useDeepSky(
		astro::catalog::DeepSkyObjectSetPtr deepsky) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got %d deepsky objects",
		deepsky->size());
	_deepsky = deepsky;
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

/**
 * \brief handle changing orientation
 *
 * The orientation parameter is true when the telescope is on the west, 
 * where the camera is oriented so that north is up on the image. When
 * the telescope is on the west side of the mount, the camera is upside
 * down, so North is on the south. On the southern hemisphere, one will
 * usually want to have this just the other way round, but that must be
 * done by the widget calling this slot, because the StarChartWidget
 * does not know where on earth the observatory is.
 *
 * \param west	orientation parameter
 */
void	StarChartWidget::orientationChanged(bool west) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got orientation change: %s",
		(west) ? "west" : "east");
	flip(!west);
	repaint();
}

void	StarChartWidget::imagerResolution(astro::Angle i) {
	_imager_resolution = i;
	repaint();
}

void	StarChartWidget::finderResolution(astro::Angle f) {
	_finder_resolution = f;
	repaint();
}

void	StarChartWidget::guiderResolution(astro::Angle g) {
	_guider_resolution = g;
	repaint();
}

void	StarChartWidget::imagerRectangle(ImagerRectangle r) {
	_imager_rectangle = r;
}

void	StarChartWidget::finderRectangle(ImagerRectangle r) {
	_finder_rectangle = r;
}

void	StarChartWidget::guiderRectangle(ImagerRectangle r) {
	_guider_rectangle = r;
}

void	StarChartWidget::setStarsVisible(bool s) {
	show_stars(s);
	repaint();
}

void	StarChartWidget::setGridVisible(bool s) {
	show_grid(s);
	repaint();
}

void	StarChartWidget::setCrosshairsVisible(bool s) {
	show_crosshairs(s);
	repaint();
}

void	StarChartWidget::setTargetVisible(bool s) {
	show_target(s);
	repaint();
}

void	StarChartWidget::setDirectionsVisible(bool s) {
	show_directions(s);
	repaint();
}

void	StarChartWidget::setDeepskyVisible(bool s) {
	show_deepsky(s);
	repaint();
}

void	StarChartWidget::setCataloglabelsVisible(bool s) {
	show_cataloglabels(s);
	repaint();
}

void	StarChartWidget::setTooltipsVisible(bool s) {
	show_tooltips(s);
	repaint();
}

void	StarChartWidget::setImagerRectangleVisible(bool s) {
	show_imager_rectangle(s);
	repaint();
}

void	StarChartWidget::setFinderRectangleVisible(bool s) {
	show_finder_rectangle(s);
	repaint();
}

void	StarChartWidget::setGuiderRectangleVisible(bool s) {
	show_guider_rectangle(s);
	repaint();
}

void	StarChartWidget::setNegative(bool s) {
	negative(s);
	repaint();
}

void	StarChartWidget::setFlip(bool s) {
	flip(s);
	repaint();
}

void	StarChartWidget::resolutionChanged(astro::Angle resolution) {
	_resolution = resolution;
	_converter = astro::ImageCoordinates(_direction, _resolution,
                        astro::Angle(0));
	repaint();
}

void	StarChartWidget::gridstepIncrement() {
	int	previous = gridstep_pixels();
	gridstep_pixels_up();
	if (previous != gridstep_pixels()) {
		repaint();
	}
}

void	StarChartWidget::gridstepDecrement() {
	int	previous = gridstep_pixels();
	gridstep_pixels_down();
	if (previous != gridstep_pixels()) {
		repaint();
	}
}

void	StarChartWidget::toggleStarsVisible() {
	setStarsVisible(!show_stars());
}

void	StarChartWidget::toggleGridVisible() {
	setGridVisible(!show_grid());
}

void	StarChartWidget::toggleCrosshairsVisible() {
	setCrosshairsVisible(!show_crosshairs());
}

void	StarChartWidget::toggleTargetVisible() {
	setTargetVisible(!show_target());
}

void	StarChartWidget::toggleDirectionsVisible() {
	setDirectionsVisible(!show_directions());
}

void	StarChartWidget::toggleDeepskyVisible() {
	setDeepskyVisible(!show_deepsky());
}

void	StarChartWidget::toggleCataloglabelsVisible() {
	setCataloglabelsVisible(!show_cataloglabels());
}

void	StarChartWidget::toggleTooltipsVisible() {
	setTooltipsVisible(!show_tooltips());
}

void	StarChartWidget::toggleNegative() {
	setNegative(!negative());
}

void	StarChartWidget::toggleFlip() {
	setFlip(!flip());
}

void	StarChartWidget::toggleImagerRectangleVisible() {
	setImagerRectangleVisible(!show_imager_rectangle());
}

void	StarChartWidget::toggleFinderRectangleVisible() {
	setFinderRectangleVisible(!show_finder_rectangle());
}

void	StarChartWidget::toggleGuiderRectangleVisible() {
	setGuiderRectangleVisible(!show_guider_rectangle());
}

void	StarChartWidget::togglePlanetsVisible() {
	setPlanetsVisible(!show_planets());
}

void	StarChartWidget::toggleMoonVisible() {
	setMoonVisible(!show_moon());
}

void	StarChartWidget::toggleSunVisible() {
	setSunVisible(!show_sun());
}

void	StarChartWidget::toggleConstellationsVisible() {
	setConstellationsVisible(!show_constellations());
}

void	StarChartWidget::useFinderResolution() {
	resolutionChanged(_finder_resolution);
}

void	StarChartWidget::useGuiderResolution() {
	resolutionChanged(_guider_resolution);
}

void	StarChartWidget::useImagerResolution() {
	resolutionChanged(_imager_resolution);
}

void	StarChartWidget::useStandardResolution() {
	resolutionChanged(_standard_resolution);
}

void	StarChartWidget::useWideResolution() {
	resolutionChanged(_wide_resolution);
}

void	StarChartWidget::setPlanetsVisible(bool p) {
	show_planets(p);
	repaint();
}

void	StarChartWidget::setSunVisible(bool p) {
	show_sun(p);
	repaint();
}

void	StarChartWidget::setMoonVisible(bool p) {
	show_moon(p);
	repaint();
}

void	StarChartWidget::setConstellationsVisible(bool p) {
	show_constellations(p);
	repaint();
}

void	StarChartWidget::increaseLimitMagnitude() {
	_limit_magnitude += 1;
	repaint();
}

void	StarChartWidget::decreaseLimitMagnitude() {
	_limit_magnitude -= 1;
	repaint();
}

time_t	StarChartWidget::time() const {
	if (_time) {
		return _time;
	}
	time_t	t;
	::time(&t);
	return t;
}

void	StarChartWidget::showContextMenu(const QPoint& point) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "show context menu at %d/%d",
		point.x(), point.y());
	QMenu	contextMenu(QString("Display options"), this);

	QAction	actionStars(QString("Stars"), this);
	actionStars.setCheckable(true);
	actionStars.setChecked(show_stars());
	contextMenu.addAction(&actionStars);
	connect(&actionStars, SIGNAL(triggered()),
		this, SLOT(toggleStarsVisible()));

	QAction	actionGrid(QString("Grid"), this);
	actionGrid.setCheckable(true);
	actionGrid.setChecked(show_grid());
	contextMenu.addAction(&actionGrid);
	connect(&actionGrid, SIGNAL(triggered()),
		this, SLOT(toggleGridVisible()));

	QAction actionGridDown(QString("Grid resolution +"), this);
	contextMenu.addAction(&actionGridDown);
	connect(&actionGridDown, SIGNAL(triggered()),
		this, SLOT(gridstepDecrement()));

	QAction actionGridUp(QString("Grid resolution -"), this);
	contextMenu.addAction(&actionGridUp);
	connect(&actionGridUp, SIGNAL(triggered()),
		this, SLOT(gridstepIncrement()));

	QAction	actionCrosshairs(QString("Crosshairs"), this);
	actionCrosshairs.setCheckable(true);
	actionCrosshairs.setChecked(show_crosshairs());
	contextMenu.addAction(&actionCrosshairs);
	connect(&actionCrosshairs, SIGNAL(triggered()),
		this, SLOT(toggleCrosshairsVisible()));

	QAction	actionTarget(QString("Target"), this);
	actionTarget.setCheckable(true);
	actionTarget.setChecked(show_target());
	contextMenu.addAction(&actionTarget);
	connect(&actionTarget, SIGNAL(triggered()),
		this, SLOT(toggleTargetVisible()));

	QAction	actionDirections(QString("Directions"), this);
	actionDirections.setCheckable(true);
	actionDirections.setChecked(show_directions());
	contextMenu.addAction(&actionDirections);
	connect(&actionDirections, SIGNAL(triggered()),
		this, SLOT(toggleDirectionsVisible()));

	QAction	actionDeepsky(QString("Deep Sky"), this);
	actionDeepsky.setCheckable(true);
	actionDeepsky.setChecked(show_deepsky());
	contextMenu.addAction(&actionDeepsky);
	connect(&actionDeepsky, SIGNAL(triggered()),
		this, SLOT(toggleDeepskyVisible()));

	QAction	actionPlanets(QString("Planets"), this);
	actionPlanets.setCheckable(true);
	actionPlanets.setChecked(show_planets());
	contextMenu.addAction(&actionPlanets);
	connect(&actionPlanets, SIGNAL(triggered()),
		this, SLOT(togglePlanetsVisible()));

	QAction	actionSun(QString("Sun"), this);
	actionSun.setCheckable(true);
	actionSun.setChecked(show_sun());
	contextMenu.addAction(&actionSun);
	connect(&actionSun, SIGNAL(triggered()),
		this, SLOT(toggleSunVisible()));

	QAction	actionMoon(QString("Moon"), this);
	actionMoon.setCheckable(true);
	actionMoon.setChecked(show_moon());
	contextMenu.addAction(&actionMoon);
	connect(&actionMoon, SIGNAL(triggered()),
		this, SLOT(toggleMoonVisible()));

	QAction	actionCataloglabels(QString("Catalog labels"), this);
	actionCataloglabels.setCheckable(true);
	actionCataloglabels.setChecked(show_cataloglabels());
	contextMenu.addAction(&actionCataloglabels);
	connect(&actionCataloglabels, SIGNAL(triggered()),
		this, SLOT(toggleCataloglabelsVisible()));

	QAction	actionConstellations(QString("Constellations"), this);
	actionConstellations.setCheckable(true);
	actionConstellations.setChecked(show_constellations());
	contextMenu.addAction(&actionConstellations);
	connect(&actionConstellations, SIGNAL(triggered()),
		this, SLOT(toggleConstellationsVisible()));

	QAction	actionTooltips(QString("Coordinates"), this);
	actionTooltips.setCheckable(true);
	actionTooltips.setChecked(show_tooltips());
	contextMenu.addAction(&actionTooltips);
	connect(&actionTooltips, SIGNAL(triggered()),
		this, SLOT(toggleTooltipsVisible()));

	contextMenu.addSeparator();

	QAction	actionNegative(QString("Negative"), this);
	actionNegative.setCheckable(true);
	actionNegative.setChecked(negative());
	contextMenu.addAction(&actionNegative);
	connect(&actionNegative, SIGNAL(triggered()),
		this, SLOT(toggleNegative()));

	QAction	actionFlip(QString("Rotate"), this);
	actionFlip.setCheckable(true);
	actionFlip.setChecked(flip());
	contextMenu.addAction(&actionFlip);
	connect(&actionFlip, SIGNAL(triggered()),
		this, SLOT(toggleFlip()));

	contextMenu.addSeparator();

	QAction actionFinderResolution(QString("Finder resolution"), this);
	if (_finder_resolution > 0) {
		actionFinderResolution.setCheckable(true);
		actionFinderResolution.setChecked(_finder_resolution == _resolution);
		contextMenu.addAction(&actionFinderResolution);
		connect(&actionFinderResolution, SIGNAL(triggered()),
			this, SLOT(useFinderResolution()));
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no finder resolution");
	}

	QAction actionGuiderResolution(QString("Guider resolution"), this);
	if (_guider_resolution > 0) {
		actionGuiderResolution.setCheckable(true);
		actionGuiderResolution.setChecked(_guider_resolution == _resolution);
		contextMenu.addAction(&actionGuiderResolution);
		connect(&actionGuiderResolution, SIGNAL(triggered()),
			this, SLOT(useGuiderResolution()));
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no guider resolution");
	}

	QAction actionImagerResolution(QString("Imager resolution"), this);
	if (_imager_resolution > 0) {
		actionImagerResolution.setCheckable(true);
		actionImagerResolution.setChecked(_imager_resolution == _resolution);
		contextMenu.addAction(&actionImagerResolution);
		connect(&actionImagerResolution, SIGNAL(triggered()),
			this, SLOT(useImagerResolution()));
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no imager resolution");
	}

	QAction actionStandardResolution(QString("Standard resolution"), this);
	actionStandardResolution.setCheckable(true);
	actionStandardResolution.setChecked(_standard_resolution == _resolution);
	contextMenu.addAction(&actionStandardResolution);
	connect(&actionStandardResolution, SIGNAL(triggered()),
		this, SLOT(useStandardResolution()));

	QAction actionWideResolution(QString("Wide resolution"), this);
	actionWideResolution.setCheckable(true);
	actionWideResolution.setChecked(_wide_resolution == _resolution);
	contextMenu.addAction(&actionWideResolution);
	connect(&actionWideResolution, SIGNAL(triggered()),
		this, SLOT(useWideResolution()));

	contextMenu.addSeparator();

	QAction	actionFinderRectangle(QString("Finder rectangle"), this);
	if (_finder_resolution > 0) {
		actionFinderRectangle.setCheckable(true);
		actionFinderRectangle.setChecked(show_finder_rectangle());
		contextMenu.addAction(&actionFinderRectangle);
		connect(&actionFinderRectangle, SIGNAL(triggered()),
			this, SLOT(toggleFinderRectangleVisible()));
	}

	QAction	actionGuiderRectangle(QString("Guider rectangle"), this);
	if (_guider_resolution > 0) {
		actionGuiderRectangle.setCheckable(true);
		actionGuiderRectangle.setChecked(show_guider_rectangle());
		contextMenu.addAction(&actionGuiderRectangle);
		connect(&actionGuiderRectangle, SIGNAL(triggered()),
			this, SLOT(toggleGuiderRectangleVisible()));
	}

	QAction	actionImagerRectangle(QString("Imager rectangle"), this);
	if (_imager_resolution > 0) {
		actionImagerRectangle.setCheckable(true);
		actionImagerRectangle.setChecked(show_imager_rectangle());
		contextMenu.addAction(&actionImagerRectangle);
		connect(&actionImagerRectangle, SIGNAL(triggered()),
			this, SLOT(toggleImagerRectangleVisible()));
	}

	contextMenu.addSeparator();

	QAction	actionLegend(QString("Legend"), this);
	contextMenu.addAction(&actionLegend);
	connect(&actionLegend, SIGNAL(triggered()),
		this, SLOT(showLegend()));

	contextMenu.addSeparator();

	std::string	s = astro::stringprintf("Limit magnitude: %.1f",
		limit_magnitude());
	QAction actionShowLimitMagnitude(QString(s.c_str()), this);
	actionShowLimitMagnitude.setEnabled(false);
	contextMenu.addAction(&actionShowLimitMagnitude);

	QAction actionIncreaseLimitMagnitude(QString("Limit Magnitude +"), this);
	//actionIncreaseLimitMagnitude.setEnabled(true);
	contextMenu.addAction(&actionIncreaseLimitMagnitude);
	connect(&actionIncreaseLimitMagnitude, SIGNAL(triggered()),
		this, SLOT(increaseLimitMagnitude()));

	QAction actionDecreaseLimitMagnitude(QString("Limit Magnitude -"), this);
	//actionDecreaseLimitMagnitude.setEnabled();
	contextMenu.addAction(&actionDecreaseLimitMagnitude);
	connect(&actionDecreaseLimitMagnitude, SIGNAL(triggered()),
		this, SLOT(decreaseLimitMagnitude()));

	QAction	actionReload(QString("Reload stars"), this);
	actionReload.setEnabled(_retriever == NULL);
	contextMenu.addAction(&actionReload);
	connect(&actionReload, SIGNAL(triggered()),
		this, SLOT(startRetrieval()));

	contextMenu.exec(mapToGlobal(point));
}

/**
 * \brief Show the legend
 */
void	StarChartWidget::showLegend() {
	if (NULL == _legend) {
		_legend = new StarChartLegend();
		connect(_legend,
			SIGNAL(destroyed()),
			this,
			SLOT(removeLegend()));
		_legend->show();
	}
	_legend->raise();
}

/**
 * \brief Remove the legend
 */
void	StarChartWidget::removeLegend() {
	_legend = NULL;
}

/**
 * \brief Compute a corner of the rectangle
 */
QPointF	StarChartWidget::rectanglePoint(const astro::RaDec& p) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "convert %f/%f with resolution %f",
		p.ra().degrees(), p.dec().degrees(), _resolution.degrees());
	float	x = p.ra() / _resolution;
	float	y = p.dec() / _resolution;
	return QPointF(_center.x() + x, _center.y() + y);
}

/**
 * \brief Draw a rectangle outlining the imager
 */
void	StarChartWidget::drawRectangle(QPainter& painter,
		const ImagerRectangle& rectangle,
		const astro::Angle& /* resolution */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "drawing rectangle %s",
		rectangle.toString().c_str());
	// set line color
	QPen	pen;
	QColor	orange(255, 102, 0);
	pen.setColor(orange);
	pen.setWidth(1);
	painter.setPen(pen);
	// set up the path for drawing
	QPainterPath	rectpath;
	float	s = 0.5;
	QPointF	p1 = rectanglePoint(rectangle.point( s,  s));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "p1 = %f,%f", p1.x(), p1.x());
	rectpath.moveTo(p1);
	QPointF	p2 = rectanglePoint(rectangle.point(-s,  s));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "p2 = %f,%f", p2.x(), p2.x());
	rectpath.lineTo(p2);
	QPointF	p3 = rectanglePoint(rectangle.point(-s, -s));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "p3 = %f,%f", p3.x(), p3.x());
	rectpath.lineTo(p3);
	QPointF	p4 = rectanglePoint(rectangle.point( s, -s));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "p4 = %f,%f", p4.x(), p4.x());
	rectpath.lineTo(p4);
	rectpath.closeSubpath();
	painter.drawPath(rectpath);
}

QPointF	StarChartWidget::position(const astro::RaDec& pos) {
	if (_direction.scalarproduct(pos) < 0) {
		throw std::runtime_error("planet on the other side");
	}
	QPointF	point = convert(pos);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "solar system body position (%f,%f)", 
		point.x(), point.y());
	if ((point.x() < 0) || (point.x() > width())) {
		throw std::runtime_error("planet outside image");
	}
	if ((point.y() < 0) || (point.y() > height())) {
		throw std::runtime_error("planet outside image");
	}
	return point;
}

/**
 * \brief Draw constellation lines
 */
void    StarChartWidget::drawConstellations(QPainter& painter) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "draw constellation lines");
	// set up the pen 
	QPen    pen(Qt::SolidLine);
	pen.setWidth(1);
	QColor  pink(255,0,204);
	pen.setColor(pink);
	painter.setPen(pen);

	// get the Constellations
	astro::catalog::ConstellationCatalogPtr consts
		= astro::catalog::ConstellationCatalog::get();
	for (auto c = consts->begin(); c != consts->end(); c++) {
		// get the next constellation
		astro::catalog::ConstellationPtr        constellation = c->second;
		for (auto e = constellation->begin();
			e != constellation->end(); e++) {
			// go through all the edges
			if ((_direction.scalarproduct(e->from()) < 0) ||
				(_direction.scalarproduct(e->to()) < 0)) {
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"point on the wrong side");
			} else {
				drawLine(painter, e->from(), e->to());
			}
		}
	}
}



} // namespace snowgui
