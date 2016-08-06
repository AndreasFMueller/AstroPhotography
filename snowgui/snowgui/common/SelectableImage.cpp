/*
 * SelectableImage.cpp -- class derived from QLabel with 
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */

#include <SelectableImage.h>
#include <QMouseEvent>
#include <AstroDebug.h>

namespace snowgui {

/**
 * \brief Construtor
 *
 * The constructor turns off all selection functions
 */
SelectableImage::SelectableImage(QWidget *parent) : QLabel(parent) {
	rubberband = new QRubberBand(QRubberBand::Rectangle, this);
	_rectangleSelectionEnabled = false;
	_pointSelectionEnabled = false;
}

/**
 * \brief Handle when the mouse is pressed
 *
 * Depending on the enabled selection, this method either starts drawing
 * a rectangle, or signals a point
 */
void	SelectableImage::mousePressEvent(QMouseEvent *e) {
	if (_rectangleSelectionEnabled) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "rectangle selection enabled");
		rubberband->show();
		origin = e->pos();
		rubberband->move(origin);
		rubberband->resize(0, 0);
	}
	if (_pointSelectionEnabled) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "emit QPoint(%d,%d)",
			e->pos().x(), e->pos().y());
		emit pointSelected(e->pos());
	}
}

/**
 * \brief Draw a rectangle
 *
 * If rectangle selection is enabled, then resize to rubberband to reflect
 * the new mouse position.
 */
void 	SelectableImage::mouseMoveEvent(QMouseEvent *e) {
	if (!_rectangleSelectionEnabled) {
		return;
	}
	int	width = e->pos().x() - origin.x();
	int	height = e->pos().y() - origin.y();
	if (width < 0) { width = 0; }
	if (height < 0) { height = 0; }
	rubberband->resize(width, height);
}

/**
 * \brief Signal a new rectangle
 *
 * If rectangle selection is enabled, signal that the rectangle has been
 * completed.
 */
void	SelectableImage::mouseReleaseEvent(QMouseEvent *e) {
	if (!_rectangleSelectionEnabled) {
		return;
	}
	int	width = e->pos().x() - origin.x();
	int	height = e->pos().y() - origin.y();
	rubberband->hide();
	if (width < 0) { return; }
	if (height < 0) { return; }
	QSize	size(width, height);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "rectangle selected: %dx%d@(%d,%d)",
		size.width(), size.height(), origin.x(), origin.y());
	QRect	rect(origin, size);
	emit rectangleSelected(rect);
}

/**
 * \brief Find out whether rectangle selection is enabled
 */
bool	SelectableImage::rectangleSelectionEnabled() {
	return _rectangleSelectionEnabled;
}

/**
 * \brief Find out whether point selection is enabled
 */
bool	SelectableImage::pointSelectionEnabled() {
	return _pointSelectionEnabled;
}

/**
 * \brief Enabled or disable rectangle selection
 */
void	SelectableImage::setRectangleSelectionEnabled(bool e) {
	_rectangleSelectionEnabled = e;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set rectangle selection to %s",
		(_rectangleSelectionEnabled) ? "yes" : "no");
	if (!e) {
		rubberband->hide();
	}
}

/**
 * \brief Enabled or disable point selection
 */
void	SelectableImage::setPointSelectionEnabled(bool e) {
	_pointSelectionEnabled = e;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set point selection to %s",
		(_pointSelectionEnabled) ? "yes" : "no");
}

} // namespace snowgui
