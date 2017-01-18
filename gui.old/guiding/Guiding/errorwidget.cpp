/*
 * errorwidget.cpp -- ErrorWidget implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <errorwidget.h>
#include <AstroDebug.h>
#include <QPainter>
#include <cmath>

void	ErrorWidget::draw() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "draw in Error widget");
	// get a QPainter
	QPainter	painter(this);
	painter.fillRect(0, 0, width(), height(), QColor(255., 255., 255.));

	// prepare the pen
	QPen	pen(Qt::SolidLine);
	pen.setWidth(1);
	pen.setColor(QColor(230., 230., 230.));
	painter.setPen(pen);
	
	// background label
	QFont	font = painter.font();
	font.setPointSize(60);
	painter.setFont(font);
	painter.drawText(0, 0, width(), height(), Qt::AlignCenter, label);

	// coordinate line
	painter.fillRect(0, height() / 2, width(), 1, QColor(128., 128., 128.));

	// time scale
	double	tmin = points.front().first;
	double	tmax = points.back().first;
	double	tscale = (width() - 2) / (tmax - tmin);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "tscale = %f", tscale);

	// error scale
	double	m = 1;
	double	y0 = points.begin()->second;
	points_t::const_iterator	i;
	for (i = points.begin(); i != points.end(); i++) {
		double	y = fabs(i->second - y0);
		if (y > m) {
			m = y;
		}
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "t = %f, y = %f", i->first, y);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "max = %f", m);
	int	zero = height() / 2;
	if (m > 1) {
		m = trunc(m) + 1;
	}
	m = (zero - 1) / m;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "zero = %f, m = %f", zero, m);

	// display the scale lines
	int	minx = -zero / m;
	int	maxx = zero / m;
	for (int x = minx; x <= maxx; x++) {
		if (x == 0) {
			continue;
		}
		int	xi = zero + m * x;
		painter.fillRect(0, xi, width(), 1, QColor(224., 224., 224.));
	}

	// draw the line
	pen.setColor(color);
	painter.setPen(pen);
	i = points.begin();
	point_t	previous;
	point_t	current = *i;
	while (points.end() != ++i) {
		// get the two points
		previous = current;
		current = *i;

		// compute the point
		QPointF	from(1 + tscale * (previous.first - tmin),
				zero - m * (previous.second - y0));
		QPointF	to(1 + tscale * (current.first - tmin),
				zero - m * (current.second - y0));
		painter.drawLine(from, to);
	}
}

void	ErrorWidget::addPoint(const point_t& point) {
	points.push_back(point);
}

void	ErrorWidget::clear() {
	points.clear();
}

void	ErrorWidget::paintEvent(QPaintEvent *event) {
	draw();
}

ErrorWidget::ErrorWidget(QWidget *parent) : QWidget(parent) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "construct an ErrorWidget");
}

ErrorWidget::~ErrorWidget() {
}

