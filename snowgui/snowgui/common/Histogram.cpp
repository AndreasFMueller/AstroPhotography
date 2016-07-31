/*
 * Histogram.cpp -- implementation of histogram container class
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "Histogram.h"
#include <QImage>
#include <QPixmap>
#include <AstroDebug.h>

namespace snowgui {

//////////////////////////////////////////////////////////////////////
// HistogramBase base class implementation
//////////////////////////////////////////////////////////////////////

HistogramBase::HistogramBase(int size) : _size(size), _logarithmic(false) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating base histogram");
	_buckets = NULL;
}

HistogramBase::~HistogramBase() {
	if (NULL != _buckets) {
		delete _buckets;
	}
}

int	HistogramBase::index(double v) {
	int	vi = v;
	if (vi >= _size) {
		return _size - 1;
	}
	if (vi < 0) {
		return 0;
	}
	return vi;
}

int	HistogramBase::bucketindex(int width, int x) const {
	double	bucketwidth = width / (double)_size;
	int	bi = x * bucketwidth;
	if (bi > _size) {
		bi = _size;
	}
	if (bi < 0) {
		bi = 0;
	}
	return bi;
}

double	HistogramBase::value(int y) const {
	if (_logarithmic) {
		if (y > 0) {
			return log10(y);
		}
		return 0;
	}
	return y;
}

//////////////////////////////////////////////////////////////////////
// monochrom histogram implementation
//////////////////////////////////////////////////////////////////////
template<>
Histogram<double>::Histogram(int size) : HistogramBase(size) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating new mono histogram");
	int	l = size;
	_buckets = new int[l];
	for (int i = 0; i < l; i++) {
		_buckets[i] = 0;
	}
}

template<>
void	Histogram<double>::add(const double& p) {
	_buckets[index(p)]++;
}

template<>
QPixmap	*Histogram<double>::pixmap(int width, int height) const {
	QImage	qimage(width, height, QImage::Format_RGB32);

	// find maximum value
	double	ymax = 0;
	for (int i = 1; i < _size - 1; i++) {
		double	v = value(_buckets[i]);
		if (v > ymax) {
			ymax = v;
		}
	}
	if (ymax == 0) {
		ymax = 1;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ymax = %f", ymax);
	double	yscale = height / ymax;

	for (int x = 0; x < width; x++) {
		int	bi = bucketindex(width, x);
		double	l = value(_buckets[bi]) * yscale;
		for (int y = 0; y < height; y++) {
			if (y <= l) {
				qimage.setPixel(x, height - 1 - y, 0xff000000);
			} else {
				qimage.setPixel(x, height - 1 - y, 0xffffffff);
			}
		}
	}

	QPixmap	*pixmap = new QPixmap(width, height);
	pixmap->convertFromImage(qimage);
	return pixmap;
}

//////////////////////////////////////////////////////////////////////
// RGB histogram implementation
//////////////////////////////////////////////////////////////////////
template<>
Histogram<RGB<double> >::Histogram(int size) : HistogramBase(size) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating new color histogram");
	int	l = 3 * size;
	_buckets = new int[l];
	for (int i = 0; i < l; i++) {
		_buckets[i] = 0;
	}
}

template<>
void	Histogram<RGB<double> >::add(const RGB<double>& p) {
	_buckets[index(p.R)               ]++;
	_buckets[index(p.G) +  _size      ]++;
	_buckets[index(p.B) + (_size << 1)]++;
}

template<>
QPixmap	*Histogram<RGB<double> >::pixmap(int width, int height) const {
	QImage	qimage(width, height, QImage::Format_RGB32);

	// find the maximum
	double	ymax = 0;
	for (int i = 1; i < _size - 1; i++) {
		double	v;
		v = value(_buckets[i               ]);
		if (v > ymax) { ymax = v; }
		v = value(_buckets[i +  _size      ]);
		if (v > ymax) { ymax = v; }
		v = value(_buckets[i + (_size << 1)]);
		if (v > ymax) { ymax = v; }
	}
	double	yscale = height / ymax;
	for (int x = 0; x < width; x++) {
		int	bi = bucketindex(width, x);
		double	lr = value(_buckets[bi]) * yscale;
		double	lg = value(_buckets[bi + _size]) * yscale;
		double	lb = value(_buckets[bi + (_size << 1)]) * yscale;
		for (int y = 0; y < height; y++) {
			unsigned int	p = 0xffffffff;
			if (y <= lr) { p -= 0x00003f3f; }
			if (y <= lg) { p -= 0x003f003f; }
			if (y <= lb) { p -= 0x003f3f00; }
			qimage.setPixel(x, height - 1 - y, p);
		}
	}

	QPixmap	*pixmap = new QPixmap(width, height);
	pixmap->convertFromImage(qimage);
	return pixmap;
}

} // namespace snowgui
