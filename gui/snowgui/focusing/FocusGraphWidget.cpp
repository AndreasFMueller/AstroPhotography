/*
 * FocusGraphWidget.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <FocusGraphWidget.h>
#include <AstroDebug.h>
#include <QPainter>

namespace snowgui {

/**
 * \brief Construct a FocusGraphWidget instance
 */
FocusGraphWidget::FocusGraphWidget(QWidget *parent) : QWidget(parent) {
	_state = snowstar::FocusIDLE;
}

/**
 * \brief Destroy the FocusGraphWidget instance
 */
FocusGraphWidget::~FocusGraphWidget() {
}

/**
 * \brief Draw the contents of the widget
 */
void	FocusGraphWidget::paintEvent(QPaintEvent * /* event */) {
	// find the minimum and the maximum
	int	minpos = _points.begin()->position;
	int	maxpos = _points.rbegin()->position;
	float	minval = _points.begin()->value;
	float	maxval = _points.begin()->value;
	std::for_each(_points.begin(), _points.end(),
		[&](const snowstar::FocusPoint& p) {
			if (p.value > maxval) {
				maxval = p.value;
			}
			if (p.value < minval) {
				minval = p.value;
			}
		}
	);

	// create a painter
	QPainter	painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	QPen	pen(Qt::SolidLine);
	pen.setColor(Qt::gray);
	painter.setPen(pen);

	// fill the background
	QColor	transparent(0, 0, 0, 0);
	painter.fillRect(0, 0, width(), height(), transparent);

	// draw a border
	QPainterPath	boundary;
	boundary.moveTo(0, 0);
	boundary.lineTo(0, height());
	boundary.lineTo(width(), height());
	boundary.lineTo(width(), 0);
	boundary.lineTo(0, 0);
	painter.drawPath(boundary);

	// if we have no points, we stop at this point
	if (_points.size() < 2) {
		return;
	}

	// compute the range that should be displayed
	float	vmin = 20;
	float	vmax = height() - 20;
	float	vscale = (vmax - vmin) / (maxpos - minpos);
	float	hmin = 5;
	float	hmax = width() - 5;
	float	hzero = 0;
	if (minval < 0) {
		if (maxval > 0) {
			float	s = (0 - minval) / (maxval - minval);
			hzero = (hmax - hmin) * s;
		} else {
			hzero = hmax;
			maxval = 0;
		}
	} else {
		// minval > 0
		hzero = hmin;
		minval = 0;
	}
	float	hscale = (hmax - hmin) / (maxval - minval);

	// draw the scale
	pen.setColor(Qt::black);
	painter.setPen(pen);
	painter.fillRect(hmin, vmin, hmax - hmin, vmax - vmin, Qt::white);
	painter.drawLine(hmin,  vmin, hmax,  vmin);
	painter.drawLine(hzero, vmin, hzero, vmax);
	painter.drawLine(hmin,  vmax, hmax,  vmax);

	// draw the position labels
	painter.drawText(hmin, vmin - 18, 100, 16,
		Qt::AlignLeft, QString::number(minpos));
	painter.drawText(hmin, vmax + 2, 100, 16,
		Qt::AlignLeft, QString::number(maxpos));

	// draw the data in a different color
	pen.setColor(Qt::blue);
	painter.setPen(pen);

	// draw the points
	QPointF	previous;
	for (auto i = _points.begin(); i != _points.end(); i++) {
		float	y = vmin + (i->position - minpos) * vscale;
		float	x = hmin + (i->value - minval) * hscale;
		QPoint	p(x, y);
		if (i != _points.begin()) {
			painter.drawLine(previous, p);
		}
		QPainterPath	path;
		path.addEllipse(x - 3, y - 3, 6, 6);
		painter.fillPath(path, Qt::blue);
		previous = p;
	}
}

/**
 * \brief Receive a new point to add to the graph
 *
 * \param p	poit to add to the list
 */
void	FocusGraphWidget::receivePoint(snowstar::FocusPoint p) {
	_points.push_back(p);
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "new point [%d] %d->%f",
	//	_points.size(), p.position, p.value);
	repaint();
}

/**
 * \brief Receive the new state
 *
 * \param s	state 
 */
void	FocusGraphWidget::receiveState(snowstar::FocusState s) {
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "new state: %d", s);
	if ((_state == snowstar::FocusIDLE)
		|| (_state == snowstar::FocusFOCUSED)
		|| (_state == snowstar::FocusFAILED)) {
		switch (s) {
		case snowstar::FocusMOVING:
		case snowstar::FocusMEASURING:
		case snowstar::FocusMEASURED:
			_points.clear();
			repaint();
			break;
		case snowstar::FocusFOCUSED:
		case snowstar::FocusFAILED:
		case snowstar::FocusIDLE:
			break;
		}
	}
	_state = s;
}

} // namespace snowgui
