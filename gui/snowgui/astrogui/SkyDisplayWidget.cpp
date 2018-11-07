/* 
 * SkyDisplayWidget.cpp -- Widget to display the visible sky
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "SkyDisplayWidget.h"
#include <AstroFormat.h>
#include <algorithm>
#include <QMouseEvent>
#include <QTimer>
#include <QToolTip>
#include <QAction>
#include <QMenu>

using namespace astro::catalog;

namespace snowgui {

static std::string	S(const astro::AzmAlt& a) {
	return astro::stringprintf("azm=%.2f,alt=%.2f", a.azm().degrees(),
		a.alt().degrees());
}

/*

The SkyDisplayWidget should have a custom context menu to turn on/off
the various features.

https://stackoverflow.com/questions/24254006/rightclick-event-in-qt-to-open-a-context-menu

customContextMenuRequested is emitted when the widget's contextMenuPolicy
is Qt::CustomContextMenu, and the user has requested a context menu
on the widget. So in the constructor of your widget you can call
setContextMenuPolicy and connect customContextMenuRequested to a
slot to make a custom context menu.

this->setContextMenuPolicy(Qt::CustomContextMenu);

connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), 
        this, SLOT(ShowContextMenu(const QPoint &)));
ShowContextMenu slot should be a class member of plotspace like :

void plotspace::ShowContextMenu(const QPoint &pos) 
{
   QMenu contextMenu(tr("Context menu"), this);

   QAction action1("Remove Data Point", this);
   connect(&action1, SIGNAL(triggered()), this, SLOT(removeDataPoint()));
   contextMenu.addAction(&action1);

   contextMenu.exec(mapToGlobal(pos));
}


http://www.setnode.com/blog/right-click-context-menus-with-qt/


*/

/**
 * \brief Construct the SkyDisplay
 *
 * \param parent	parent widget
 */
SkyDisplayWidget::SkyDisplayWidget(QWidget *parent) : QWidget(parent) {
	qRegisterMetaType<astro::catalog::Catalog::starsetptr>("astro::catalog::Catalog::starsetptr");

	// get all the stars from the BSC catalog
	SkyStarThread	*_skystarthread = new SkyStarThread(this);
	connect(_skystarthread,
		SIGNAL(stars(astro::catalog::Catalog::starsetptr)),
		this,
		SLOT(useStars(astro::catalog::Catalog::starsetptr)));
	connect(_skystarthread, SIGNAL(finished()),
		_skystarthread, SLOT(deleteLater()));
	_skystarthread->start();

	// context menu
	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), 
		this, SLOT(showContextMenu(const QPoint &)));

	_show_tooltip = true;
	_mouse_pressed = false;

	// start the update timer
	_timer.setInterval(60000);
	connect(&_timer, SIGNAL(timeout()),
		this, SLOT(update()));
	_timer.start();

	// enable mouse tracking
	setMouseTracking(true);
}

/**
 * \brief Destroy the SkyDisplay
 */
SkyDisplayWidget::~SkyDisplayWidget() {
	_timer.stop();
}

/**
 * \brief  Paint event handler
 */
void	SkyDisplayWidget::paintEvent(QPaintEvent * /* event */) {
	QPainter	painter(this);
	QSize	s = size();
	draw(painter, s);
}

astro::RaDec	SkyDisplayWidget::convert(QMouseEvent *e) {
	double	deltax = e->pos().x() - _center.x();
	double	deltay = e->pos().y() - _center.y();

	// compute the radius
	double	f = hypot(deltax, deltay) / _radius;
	if (f > 1) {
		throw std::range_error("outside circle");
	}

	// convert the radius to an angle
	astro::Angle	s((1 - f) * M_PI / 2);

	astro::AzmAlt	azmalt(astro::arctan2(deltax, deltay), s);
	
	debug(LOG_DEBUG, DEBUG_LOG, 0, "azm = %.3f, alt = %.3f",
		azmalt.azm().degrees(), azmalt.alt().degrees());

	// convert the to RaDec
	return _converter->inverse(azmalt);
}

void	SkyDisplayWidget::redraw() {
	SkyDrawing::redraw();
	repaint();
}

/**
 * \brief All mouse events are processed the same way with this method
 *
 * \param e	the mouse event to process
 */
void	SkyDisplayWidget::mouseCommon(const astro::RaDec& _target) {
	target(_target);

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
	try {
		astro::RaDec	t = convert(e);
		mouseCommon(t);
	} catch (...) { }
}

/**
 * \brief Mouse release event
 */
void	SkyDisplayWidget::mouseReleaseEvent(QMouseEvent * /* e */) {
	_mouse_pressed = false;
}

/**
 * \brief Mouse move event
 *
 * computes the coordinates where the mouse was pressed and emits them
 * with the signal pointSelected(astro::RaDec)
 */
void	SkyDisplayWidget::mouseMoveEvent(QMouseEvent *event) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "mouseMoveEvent %d,%d",
		event->pos().x(), event->pos().y());
	try {
		astro::RaDec	t = convert(event);
		if (_mouse_pressed) {
			mouseCommon(t);
		}
		while (t.ra() < astro::Angle(0)) {
			t.ra() = t.ra() + astro::Angle(360,
						astro::Angle::Degrees);
		}
		QString tiptext(astro::stringprintf("RA: %s DEC: %s",
				t.ra().hms(':', -1).c_str(),
				t.dec().dms(':', -1).c_str()).c_str());
		QToolTip::showText(event->globalPos(), tiptext);
	} catch (...) { }
}

/**
 * \brief Ensure that the object is deleted when it is hit by a close event
 */
void	SkyDisplayWidget::closeEvent(QCloseEvent * /* event */) {
	deleteLater();
}

/**
 * \brief Slot to trigger a redrawing
 */
void	SkyDisplayWidget::update() {
	repaint();
}

void	SkyDisplayWidget::useStars(Catalog::starsetptr stars) {
	SkyDrawing::useStars(stars);
}

void	SkyDisplayWidget::telescopeChanged(astro::RaDec radec) {
	SkyDrawing::telescopeChanged(radec);
}

void	SkyDisplayWidget::targetChanged(astro::RaDec radec) {
	SkyDrawing::targetChanged(radec);
}

void	SkyDisplayWidget::positionChanged(astro::LongLat longlat) {
	SkyDrawing::positionChanged(longlat);
}

void    SkyDisplayWidget::setAltAzmGridVisible(bool s) {
	show_altaz(s);
	repaint();
}

void    SkyDisplayWidget::setRaDecGridVisible(bool s) {
	show_radec(s);
	repaint();
}

void    SkyDisplayWidget::setConstellationsVisible(bool s) {
	show_constellations(s);
	repaint();
}

void    SkyDisplayWidget::setTargetVisible(bool s) {
	show_target(s);
	repaint();
}

void    SkyDisplayWidget::setTelescopeVisible(bool s) {
	show_telescope(s);
	repaint();
}

void    SkyDisplayWidget::setTooltipVisible(bool s) {
	show_tooltip(s);
	repaint();
}

void    SkyDisplayWidget::setLabelsVisible(bool s) {
	show_labels(s);
	repaint();
}

void    SkyDisplayWidget::setEclipticVisible(bool s) {
	show_ecliptic(s);
	repaint();
}

void	SkyDisplayWidget::toggleRaDecGridVisible() {
	setRaDecGridVisible(!show_radec());
}

void	SkyDisplayWidget::toggleAltAzmGridVisible() {
	setAltAzmGridVisible(!show_altaz());
}

void	SkyDisplayWidget::toggleConstellationsVisible() {
	setConstellationsVisible(!show_constellations());
}

void	SkyDisplayWidget::toggleTargetVisible() {
	setTargetVisible(!show_target());
}

void	SkyDisplayWidget::toggleTelescopeVisible() {
	setTelescopeVisible(!show_telescope());
}

void	SkyDisplayWidget::toggleTooltipVisible() {
	setTooltipVisible(!show_tooltip());
}

void	SkyDisplayWidget::toggleLabelsVisible() {
	setLabelsVisible(!show_labels());
}

void	SkyDisplayWidget::toggleEclipticVisible() {
	setEclipticVisible(!show_ecliptic());
}

void	SkyDisplayWidget::showContextMenu(const QPoint& point) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "show context menu at %d/%d",
		point.x(), point.y());
	QMenu contextMenu("Options:", this);

	QAction	actionRaDec(QString("RA/DEC grid"), this);
	actionRaDec.setCheckable(true);
	actionRaDec.setChecked(show_radec());
	contextMenu.addAction(&actionRaDec);
	connect(&actionRaDec, SIGNAL(triggered()),
		this, SLOT(toggleRaDecGridVisible()));

	QAction	actionAzmAlt(QString("Azm/Alt grid"), this);
	actionAzmAlt.setCheckable(true);
	actionAzmAlt.setChecked(show_altaz());
	contextMenu.addAction(&actionAzmAlt);
	connect(&actionAzmAlt, SIGNAL(triggered()),
		this, SLOT(toggleAltAzmGridVisible()));

	QAction	actionEcliptic(QString("Ecliptic"), this);
	actionEcliptic.setCheckable(true);
	actionEcliptic.setChecked(show_ecliptic());
	contextMenu.addAction(&actionEcliptic);
	connect(&actionEcliptic, SIGNAL(triggered()),
		this, SLOT(toggleEclipticVisible()));

	QAction	actionConstellations(QString("Constellations"), this);
	actionConstellations.setCheckable(true);
	actionConstellations.setChecked(show_constellations());
	contextMenu.addAction(&actionConstellations);
	connect(&actionConstellations, SIGNAL(triggered()),
		this, SLOT(toggleConstellationsVisible()));

	QAction	actionTelescope(QString("Telescope position"), this);
	actionTelescope.setCheckable(true);
	actionTelescope.setChecked(show_telescope());
	contextMenu.addAction(&actionTelescope);
	connect(&actionTelescope, SIGNAL(triggered()),
		this, SLOT(toggleTelescopeVisible()));

	QAction	actionTarget(QString("Target"), this);
	actionTarget.setCheckable(true);
	actionTarget.setChecked(show_target());
	contextMenu.addAction(&actionTarget);
	connect(&actionTarget, SIGNAL(triggered()),
		this, SLOT(toggleTargetVisible()));

	QAction	actionLabels(QString("Direction labels"), this);
	actionLabels.setCheckable(true);
	actionLabels.setChecked(show_labels());
	contextMenu.addAction(&actionLabels);
	connect(&actionLabels, SIGNAL(triggered()),
		this, SLOT(toggleLabelsVisible()));

	contextMenu.exec(mapToGlobal(point));
}


} // namespace snowgui


