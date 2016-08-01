/*
 * SelectableImage.cpp -- class derived from QLabel with 
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */

#include <SelectableImage.h>
#include <QMouseEvent>
#include <AstroDebug.h>

namespace snowgui {

SelectableImage::SelectableImage(QWidget *parent) : QLabel(parent) {
	rubberband = new QRubberBand(QRubberBand::Rectangle, this);
	_rectangleSelectionEnabled = false;
	_pointSelectionEnabled = false;
}

void	SelectableImage::mousePressEvent(QMouseEvent *e) {
	if (_rectangleSelectionEnabled) {
		rubberband->show();
		origin = e->pos();
		rubberband->move(origin);
		rubberband->resize(0, 0);
		return;
	}
	if (_pointSelectionEnabled) {
		emit pointSelected(e->pos());
	}
}

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

bool	SelectableImage::rectangleSelectionEnabled() {
	return _rectangleSelectionEnabled;
}

bool	SelectableImage::pointSelectionEnabled() {
	return _pointSelectionEnabled;
}

void	SelectableImage::setRectangleSelectionEnabled(bool e) {
	if (e) {
		_pointSelectionEnabled = false;
	} else {
		rubberband->hide();
	}
	_rectangleSelectionEnabled = e;
}

void	SelectableImage::setPointSelectionEnabled(bool e) {
	if (e) {
		_rectangleSelectionEnabled = false;
		rubberband->hide();
	}
	_pointSelectionEnabled = e;
}

} // namespace snowgui
