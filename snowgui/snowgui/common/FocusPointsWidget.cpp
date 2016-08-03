/*
 * FocusPointsWidget.cpp -- widget to display focus points
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */

#include <FocusPointsWidget.h>
#include <AstroDebug.h>
#include <QPainter>
#include <QMouseEvent>
#include <QToolTip>

using namespace astro::image;

namespace snowgui {

/**
 * \brief Construct a widget
 */
FocusPointsWidget::FocusPointsWidget(QWidget *parent) : QWidget(parent) {
	_byposition = true;
	setMouseTracking(true);
}

/**
 * \brief Destroy the widget
 */
FocusPointsWidget::~FocusPointsWidget() {
}

/**
 * \brief Add a new image and position to the focuspoints
 */
void	FocusPointsWidget::add(ImagePtr image, unsigned short position) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adding new focus point");
	_focuspoints.add(image, position);
	repaint();
}

/**
 * \brief Draw a list of points
 */
void	FocusPointsWidget::drawPoints(QPainter& painter,
		const Scaler::pointlist& pl) {
	std::for_each(pl.begin(), pl.end(),
		[&painter](const Scaler::point& p) {
			painter.drawPoint(QPoint(p.x(), p.y()));
		}
	);

	QPen	pen(Qt::SolidLine);
	pen.setWidth(1);
	pen.setColor(QColor(0., 0., 255.));
	painter.setPen(pen);

	Scaler::pointlist::const_iterator	i;
	i = pl.begin();
	Scaler::point	p1 = *i;
	i++;
	while (i != pl.end()) {
		painter.drawLine(QPoint(p1.x(), p1.y()),
			QPoint(i->x(), i->y()));
		p1 = *i;
		i++;
	}
}

/**
 * \brief Draw the focus points by position
 */
void	FocusPointsWidget::drawByPosition(QPainter& painter) {
	int	minposition = _focuspoints.minposition();
	int	maxposition = _focuspoints.maxposition();
	scaler = Scaler(minposition, maxposition,
			_focuspoints.minfocus(), _focuspoints.maxfocus(),
			width(), height(), 20);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d points, scaler: %s",
		_focuspoints.size(), scaler.toString().c_str());
	Scaler::pointlist	pl
		= scaler.listWithPosition(_focuspoints.sortByPosition());
	drawPoints(painter, pl);

	if (_focuspoints.size() < 2) {
		return;
	}
	QRect	r(QPoint(3, height() - 15 - 3), QSize(width() - 6, 20));
	QString	leftlabel = QString::number(minposition);
	painter.drawText(r, Qt::AlignLeft, leftlabel);
	QString	rightlabel = QString::number(maxposition);
	painter.drawText(r, Qt::AlignRight, rightlabel);
}

/**
 * \brief Draw the focus points by sequence
 */
void	FocusPointsWidget::drawBySequence(QPainter& painter) {
	int	minsequence = _focuspoints.minsequence();
	int	maxsequence = _focuspoints.maxsequence();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "range: %d - %d",
		minsequence, maxsequence);
	scaler = Scaler(minsequence, maxsequence,
			_focuspoints.minfocus(), _focuspoints.maxfocus(),
			width(), height(), 0);
	Scaler::pointlist	pl
		= scaler.listWithSequence(_focuspoints.sortBySequence());
	drawPoints(painter, pl);
}

/**
 * \brief Common draw function
 */
void	FocusPointsWidget::draw() {
	QPainter	painter(this);
	painter.fillRect(0, 0, width(), height(), QColor(255., 255., 255.));
	QPen	pen(Qt::SolidLine);
	pen.setWidth(3);
	pen.setColor(QColor(0., 0., 255.));
	painter.setPen(pen);

	if (_byposition) {
		drawByPosition(painter);
	} else {
		drawBySequence(painter);
	}
}

/**
 * \brief Paint event to initiate drawing
 */
void	FocusPointsWidget::paintEvent(QPaintEvent * /* event */) {
	draw();
}

/**
 * \brief Common utility function to display the position as a tooltip
 */
int	FocusPointsWidget::showPositionAsTooltip(QMouseEvent *event) {
	int	position = scaler.inverse(event->pos()).x();
	QToolTip::showText(event->globalPos(),
		QString::number(position),
		this, rect() );
	QWidget::mouseMoveEvent(event);
	return position;
}

/**
 * \brief Mouse press event handler
 *
 * This handler emits the positionSelected signal when user selects a
 * position in the image
 */
void	FocusPointsWidget::mousePressEvent(QMouseEvent *event) {
	if (!_byposition) {
		return;
	}
	if (_focuspoints.size() < 2) {
		return;
	}
	int	position = showPositionAsTooltip(event);
	emit positionSelected(position);
}

/**
 * \brief Mouse move event handler
 *
 * This handler tracks the position of the mouse, converts it to
 * a position and displays the position as a tooltip using the
 * showPositionAsTooltip method.
 */
void	FocusPointsWidget::mouseMoveEvent(QMouseEvent *event) {
	if (!_byposition) {
		return;
	}
	if (_focuspoints.size() < 2) {
		return;
	}
	showPositionAsTooltip(event);
}

/**
 * \brief Clear the set of focus points
 */
void	FocusPointsWidget::clear() {
	_focuspoints.clear();
	repaint();
}

/**
 * \brief Switch between position/sequence display
 */
void	FocusPointsWidget::setByPosition(bool b) {
	if (b == _byposition) {
		return;
	}
	_byposition = b;
	repaint();
}

} // namespace snowgui
