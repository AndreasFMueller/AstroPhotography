/*
 * guidehistorywidget.cpp -- widget for displaying tracking error history
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <guidehistorywidget.h>
#include <AstroDebug.h>
#include <QPainter>

/**
 * \brief Create a GuideHistoryWidget
 */
GuideHistoryWidget::GuideHistoryWidget(QWidget *parent) : QWidget(parent) {
#if 0
	// add fake data just to have something to display
	for (int x = 0; x < 400; x++) {
		data.push_back(sin(0.05 * x));
	}
#endif
}

/**
 * \brief Destroy the GuideHistoryWidget
 */
GuideHistoryWidget::~GuideHistoryWidget() {
}

void	GuideHistoryWidget::paintEvent(QPaintEvent *event) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "paint event");
	drawCurve();
}

/**
 * \brief Draw the collected data as a curve
 */
void	GuideHistoryWidget::drawCurve() {
	// for the drawing, we need a pointer
	QPainter	painter(this);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got a QPainter");
	painter.fillRect(0, 0, width(), height(), QColor(255., 255., 255.));
	int	zero = round(height() / 2);
	painter.fillRect(0, zero, width(), 1, QColor(0., 0., 0.));
	painter.fillRect(0, 1, 1, height() + 1, QColor(0., 0., 0.));

	// display data
	debug(LOG_DEBUG, DEBUG_LOG, 0, "drawing curve with %d points",
		data.size());
	if (data.size() <= 1) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no data to draw");
		return;
	}
	// first determine maximum and minimum in the dequeue
	double	max = 0;
	double	x = 0, x2 = 0;
	std::deque<double>::const_iterator	i;
	for (i = data.begin(); i != data.end(); i++) {
		double	v = *i;
		x += v;
		x2 += v * v;
		v = fabs(v);
		if (v > max) { max = v; }
	}
	if (max < 1) {
		max = 1;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "maximum: %f", max);

	// determine the scale factors to compute y coordinates from values
	// y = a * x + b
	b = height() / 2;
	a = b / max;

	// compute mean and variance
	int	n = data.size();
	double	mean = x / n;
	double	stdev = sqrt(n * (x2 / n - mean * mean) / (n - 1));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "mean: %f, variance: %f", mean, stdev);

	// draw the vertical axes ticks
	int	m = ceil(b / a);
	for (int yi = -m; yi <= m; yi++) {
		if (yi != 0) {
			int	yl = round(ycorr(yi));
			if ((yl < height()) && (yl >= 0)) {
				painter.fillRect(0, yl, 3, 1, QColor(0., 0., 0.));
			}
		}
	}

	// draw zero line and variance strip
	int	yl = round(ycorr(mean));
	painter.fillRect(1, yl, width() - 1, 1, QColor(128., 128., 128.));
	yl = round(ycorr(mean + stdev));
	painter.fillRect(1, yl, width() - 1, 1, QColor(196., 196., 196.));
	yl = round(ycorr(mean - stdev));
	painter.fillRect(1, yl, width() - 1, 1, QColor(196., 196., 196.));

	// draw curve
	double	previous;
	std::deque<double>::const_reverse_iterator	j = data.rbegin();
	previous = *j;
	int	counter = 0;
	painter.setPen(color());
	while (((counter++) < width()) && (++j != data.rend())) {
		QPoint	from(width() - counter + 1, round(ycorr(previous)));
		QPoint	to(width() - counter, round(ycorr(*j)));
		painter.drawLine(from, to);
		previous = *j;
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "redraw complete");
}

/**
 * \brief Add a single value to the history
 */
void	GuideHistoryWidget::add(double value) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new value: %f", value);
	data.push_back(value);
	std::deque<double>::size_type	l = this->width();
	while (data.size() > l) {
		data.pop_front();
	}
	// trigger redrawing of the curve
	repaint();
}

