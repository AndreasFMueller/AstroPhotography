/*
 * AstroHistogram.h -- Computation of Histograms of images
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroHistogram_h
#define _AstroHistogram_h

#include <AstroImage.h>
#include <AstroDebug.h>

namespace astro {
namespace image {

class HistogramScale {
	unsigned int	_buckets;
public:
	HistogramScale(unsigned int buckets);
	unsigned int	buckets() const { return _buckets; }
	virtual double	left(unsigned int i) const = 0;
	virtual double	right(unsigned int i) const = 0;
	virtual unsigned int	bucket(double v) const = 0;
	double	min() const;
	double	max() const;
	virtual std::string	toString() const;
};

class HistogramLinearScale : public HistogramScale {
	double	_min;
	double	_max;
	double	step;
public:
	HistogramLinearScale(double min, double max, unsigned int buckets);
	virtual double	left(unsigned int i) const;
	virtual double	right(unsigned int i) const;
	virtual unsigned int	bucket(double v) const;
	virtual std::string	toString() const;
};

typedef std::tr1::shared_ptr<HistogramScale>	HistogramScalePtr;

namespace histogram {
typedef enum {	LUMINANCE, RED, GREEN, BLUE } type;
} // namespace histogram

class HistogramScaleFactory {
	ImagePtr	image;
public:
	HistogramScaleFactory(ImagePtr _image) : image(_image) { }
	HistogramScalePtr	operator()(const histogram::type& channel, unsigned int buckets);
};

class HistogramBase {
protected:
	HistogramScalePtr	scale;
	std::tr1::shared_ptr<unsigned int>	counts;
	unsigned int	*p;
public:
	HistogramBase(HistogramScalePtr _scale);
	unsigned int	buckets() const;
	double	min() const;
	double	max() const;
	unsigned int	bucket(double v) const;
	double	left(unsigned int i) const;
	double	right(unsigned int i) const;
	unsigned int	count(unsigned int i) const;
	unsigned int	maxcount() const;
	virtual std::string	toString() const;
};

typedef std::tr1::shared_ptr<HistogramBase>	HistogramPtr;

class HistogramFactory {
	HistogramScalePtr	scale;
public:
	HistogramFactory(HistogramScalePtr _scale) : scale(_scale) { }
	HistogramPtr	operator()(ImagePtr image,
				const histogram::type& channel) const;
};

class HistogramSet {
	HistogramPtr	get(ImagePtr image, const histogram::type& channel,
				unsigned int buckets) const;
public:
	HistogramPtr	luminance;
	HistogramPtr	red;
	HistogramPtr	green;
	HistogramPtr	blue;
	HistogramSet() { }
	HistogramSet(ImagePtr image, unsigned int buckets);
};

} // namespace image
} // namespace astro

#endif /* _AstroHistogram_h */
