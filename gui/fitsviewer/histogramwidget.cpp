/*
 * histogramwidget.cpp -- HistogramWidget implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "histogramwidget.h"
#include <math.h>
#include <AstroDebug.h>
#include <QPainter>
#include <QPaintEvent>

using namespace astro::image;

/**
 * \brief Construct an HistogramWidget
 */
HistogramWidget::HistogramWidget(QWidget *parent) :
    QWidget(parent)
{
	logarithmic = true;
	color = false;
	update();
	setBackgroundRole(QPalette::Base);
	setAutoFillBackground(true);
	_minmark = -1;
	_maxmark = -1;
}

/**
 * \brief Destroy the HistogramWidet
 */
HistogramWidget::~HistogramWidget()
{
}


void	HistogramWidget::setHistograms(HistogramSet _histogramset) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setting new histogram: %u",
		_histogramset.luminance->buckets());
	histogramset = _histogramset;
	_min = histogramset.luminance->min();
	_max = histogramset.luminance->max();
	if (_minmark < 0) {
		_minmark = _min;
	}
	if (_maxmark < 0) {
		_maxmark = _max;
	}
	update();
}

void	HistogramWidget::update() {
	if (!histogramset.luminance) {
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "histogram update: %u",
		histogramset.luminance->buckets());

	// first find the maximum of the values of the histogram
	if (color) {
		maxcount = histogramset.red->maxcount();
		unsigned int	newcount = histogramset.green->maxcount();
		if (newcount > maxcount) { maxcount = newcount; }
		newcount = histogramset.blue->maxcount();
		if (newcount > maxcount) { maxcount = newcount; }
	} else {
		maxcount = histogramset.luminance->maxcount();
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "maxcount complete: %u", maxcount);
	
	// actually display the stuff
	QWidget::update();
}

void	HistogramWidget::drawLuminance() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "drawing: %u",
		histogramset.luminance->buckets());
	QPainter	painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	double	step = width() / (double)histogramset.luminance->buckets();
	double	maxvalue = histogramset.luminance->maxcount();
	double	hzero = height() - 10;
	double	vscale;
	if (logarithmic) {
		vscale = hzero / log10(maxvalue);
	} else {
		vscale = hzero / maxvalue;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "step = %f, vscale = %f", step, vscale);
	for (unsigned int bucket = 0;
		bucket < histogramset.luminance->buckets(); bucket++) {
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "bucket %d", bucket);
		unsigned int	c = histogramset.luminance->count(bucket);
		double	v = c;
		if ((c > 0) && (logarithmic)) {
			v = log10(c);
		}
		double	displaystep = (step > 1) ? step : 1.;
		QRect	rectangle(bucket * step, hzero - vscale * v,
				displaystep, vscale * v);
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "%.1f, %.1f, %.1f, %.1f",
		//	bucket * step, hzero - vscale * v,
		//	displaystep, vscale * v);
		painter.fillRect(rectangle, QColor(0., 0., 0.));
	}

	unsigned int	minx = histogramset.luminance->bucket(_minmark);
	QPointF	minpoints[3] = {
		QPointF(minx * step, hzero),
		QPointF(minx * step, hzero + 10),
		QPointF(minx * step + 10, hzero + 10)
	};
	painter.drawConvexPolygon(minpoints, 3);

	unsigned int	maxx = histogramset.luminance->bucket(_maxmark);
	QPointF	maxpoints[3] = {
		QPointF(maxx * step, hzero),
		QPointF(maxx * step, hzero + 10),
		QPointF(maxx * step - 10, hzero + 10)
	};
	painter.drawConvexPolygon(maxpoints, 3);
}

void	HistogramWidget::drawColor() {
	debug(LOG_ERR, DEBUG_LOG, 0, "color histogram not implemented");
}

void	HistogramWidget::paintEvent(QPaintEvent *event) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "paint event");
	if (!histogramset.luminance) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no histogram");
		return;
	}
	if (color) {
		drawColor();
	} else {
		drawLuminance();
	}
}
