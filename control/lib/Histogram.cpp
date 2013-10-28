/*
 * Histogram.cpp --
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroHistogram.h>
#include <AstroFormat.h>
#include <AstroAdapter.h>
#include <AstroFilterfunc.h>
#include <sstream>

using namespace astro::image;
using namespace astro::image::filter;

namespace astro {
namespace image {

//////////////////////////////////////////////////////////////////////
// Histgram Scale
//////////////////////////////////////////////////////////////////////

HistogramScale::HistogramScale(unsigned int buckets) : _buckets(buckets) { }

double	HistogramScale::min() const {
	return this->left(0);
}

double	HistogramScale::max() const {
	return this->right(_buckets);
}

std::string	HistogramScale::toString() const {
	return stringprintf("%d-Histogram", _buckets);
}

//////////////////////////////////////////////////////////////////////
// Linear Histgram Scale
//////////////////////////////////////////////////////////////////////

HistogramLinearScale::HistogramLinearScale(double min, double max,
	unsigned int buckets)
	: HistogramScale(buckets), _min(min), _max(max) {
	step = (_max - _min) / buckets;
}

double	HistogramLinearScale::left(unsigned int i) const {
	if (i > buckets()) {
		std::range_error("bucket index exceeds bucket number");
	}
	return _min + i * step;
}

double	HistogramLinearScale::right(unsigned int i) const {
	if (i > buckets()) {
		std::range_error("bucket index exceeds bucket number");
	}
	return _min + (i + 1) * step;
}

unsigned int	HistogramLinearScale::bucket(double v) const {
	int	b = trunc(v - _min) / step;
	if (b < 0) {
		return 0;
	}
	unsigned int	bb = b;
	if (bb >= buckets()) {
		return buckets() - 1;
	}
	return b;
}

std::string	HistogramLinearScale::toString() const {
	return stringprintf("%d-histogram [%.3f, %.3f]",
		buckets(), min(), max());
}

//////////////////////////////////////////////////////////////////////
// HistogramBase
//////////////////////////////////////////////////////////////////////
HistogramBase::HistogramBase(HistogramScalePtr _scale) : scale(_scale) {
	p = new unsigned int[scale->buckets()];
	for (unsigned int i = 0; i < scale->buckets(); i++) {
		p[i] = 0;
	}
	counts = std::shared_ptr<unsigned int>(p);
debug(LOG_DEBUG, DEBUG_LOG, 0, "buckets: %d", buckets());
}

unsigned int	HistogramBase::buckets() const {
	return scale->buckets();
}

double	HistogramBase::min() const {
	return scale->min();
}

double	HistogramBase::max() const {
	return scale->max();
}

unsigned int	HistogramBase::bucket(double v) const {
	return scale->bucket(v);
}

double	HistogramBase::left(unsigned int i) const {
	return scale->left(i);
}

double	HistogramBase::right(unsigned int i) const {
	return scale->right(i);
}

unsigned int	HistogramBase::count(unsigned int i) const {
	return p[i];
}

std::string	HistogramBase::toString() const {
	std::stringstream	out;
	for (unsigned int i = 0; i < buckets(); i++) {
		std::string	m = stringprintf("[%.3f, %.3f] = %d",
			left(i), right(i), count(i));
		out << m << std::endl;
	}
	return out.str();
}

unsigned int	HistogramBase::maxcount() const {
	unsigned int	result = 0;
	for (unsigned int i = 0; i < buckets(); i++) {
		unsigned int	newcount = count(i);
		if (newcount > result) {
			result = newcount;
		}
	}
	return result;
}

//////////////////////////////////////////////////////////////////////
// Derived histogram classes
//////////////////////////////////////////////////////////////////////

template<typename Pixel>
class Histogram : public HistogramBase {
public:
	Histogram(HistogramScalePtr scale,
		const ConstImageAdapter<Pixel>& image);
};

template<typename Pixel>
Histogram<Pixel>::Histogram(HistogramScalePtr _scale,
	const ConstImageAdapter<Pixel>& image)
	: HistogramBase(_scale) {
	// image dimensions
	unsigned int	width = image.getSize().width();
	unsigned int	height = image.getSize().height();

	// count values
	for (unsigned int x = 0; x < width; x++) {
		for (unsigned int y = 0; y < height; y++) {
			// get the value of the present pixel
			double	value = image.pixel(x, y);

			// don't count points that exceed the maximum
			if (value >= scale->max()) {
				continue;
			}
			// don't count points that are below the minimum
			if (value < scale->min()) {
				continue;
			}
			// find out which bucket this value belongs to and count
			int	b = scale->bucket(value);
			p[b]++;
		}
	}
}

template<typename P, typename T>
class HistogramValueAdapter : public ConstImageAdapter<T> {
	const ConstImageAdapter<RGB<P> >&	_image;
	histogram::type	_channel;
public:
	HistogramValueAdapter(const ConstImageAdapter<RGB<P> >& image,
		const histogram::type& channel)
		: ConstImageAdapter<T>(image.getSize()),
		  _image(image), _channel(channel) {
	}
	virtual const T	pixel(unsigned int x, unsigned int y) const {
		RGB<P>	v = _image.pixel(x, y);
		switch (_channel) {
		case histogram::LUMINANCE:
			return (T)luminance(v);
		case histogram::RED:
			return (T)v.R;
		case histogram::GREEN:
			return (T)v.G;
		case histogram::BLUE:
			return (T)v.B;
		}
		return 0;
	}
};

//////////////////////////////////////////////////////////////////////
// Histogram factory
//////////////////////////////////////////////////////////////////////
HistogramPtr	HistogramFactory::operator()(ImagePtr image,
	const histogram::type& channel) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating histogram with scale %s",
		scale->toString().c_str());
	HistogramBase	*histogram = NULL;
	{
		Image<RGB<float> >	*imagep
			= dynamic_cast<Image<RGB<float> > *>(&*image);
		if (NULL != imagep) {
			HistogramValueAdapter<float, float> va(*imagep, channel);
			histogram = new Histogram<float>(scale, va);
		}
	}
	{
		Image<RGB<double> >	*imagep
			= dynamic_cast<Image<RGB<double> > *>(&*image);
		if (NULL != imagep) {
			HistogramValueAdapter<double, double>	va(*imagep, channel);
			histogram = new Histogram<double>(scale, va);
		}
	}
	if (NULL == histogram) {
		throw std::runtime_error("cannot produce histograms for this image type");
	}
	return HistogramPtr(histogram);
}

//////////////////////////////////////////////////////////////////////
// HistogramScaleFactory
//
// For this factory we need a few auxiliary classes to extract maximum
// and minimum pixel value or luminance value
//////////////////////////////////////////////////////////////////////
HistogramScalePtr	HistogramScaleFactory::operator()(
	const histogram::type& channel, unsigned int buckets) {
	double	min = 0;
	double	max = 0;
	switch (channel) {
	case histogram::LUMINANCE:
		max = max_luminance(image);
		min = min_luminance(image);
		break;
	case histogram::RED:
	case histogram::GREEN:
	case histogram::BLUE:
		max = max_RGB(image);
		max = max_RGB(image);
		break;
	}
	return HistogramScalePtr(new HistogramLinearScale(min, max, buckets));
}

//////////////////////////////////////////////////////////////////////
// Histogram set of an image
//////////////////////////////////////////////////////////////////////
HistogramPtr	HistogramSet::get(ImagePtr image,
			const histogram::type& channel, unsigned int buckets) const {
	HistogramScaleFactory	sf(image);
	HistogramScalePtr	scale = sf(channel, buckets);
	HistogramFactory	hf(scale);
	return hf(image, histogram::LUMINANCE);
}

HistogramSet::HistogramSet(ImagePtr image, unsigned int buckets) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get luminance histogram");
	luminance = get(image, histogram::LUMINANCE, buckets);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get red histogram");
	red = get(image, histogram::RED, buckets);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get green histogram");
	green = get(image, histogram::GREEN, buckets);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get blue histogram");
	blue = get(image, histogram::BLUE, buckets);
}

} // namespace image
} // namespace astro
