/*
 * Histogram.h -- basic histogram class
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Histogram_h
#define _Histogram_h

#include <QPixmap>
#include <AstroPixel.h>

using namespace astro::image;

namespace snowgui {

/**
 * \brief base class for histogram computation
 */
class HistogramBase {
protected:
	int	_size;
	int	*_buckets;
	bool	_logarithmic;
	int	index(double v);
	int	bucketindex(int width, int x) const;
	double	value(int y) const;
public:
	bool	logarithmic() const { return _logarithmic; }
	void	logarithmic(bool l) { _logarithmic = l; }
	HistogramBase(int size);
	virtual	~HistogramBase();
	virtual QPixmap	*pixmap(int width, int height) const = 0;
};
typedef std::shared_ptr<HistogramBase>	HistogramPtr;

/**
 * \brief template for pixel type dependent histogram computation
 */
template<typename Pixel>
class Histogram : public HistogramBase {
	// prevent copying
private:
	Histogram(const Histogram<Pixel>& other);
	Histogram<Pixel>&	operator=(const Histogram<Pixel>& other);
public:
	Histogram(int size = 256) : HistogramBase(size) { }
	virtual void	add(const Pixel& p);
	virtual QPixmap	*pixmap(int /* width */, int /* height */) const {
		return NULL;
	}
};

// specializations for monochrom images
template<>
Histogram<double>::Histogram(int size);

template<>
void	Histogram<double>::add(const double& p);

template<>
QPixmap	*Histogram<double>::pixmap(int width, int height) const;

// specializations for color images
template<>
Histogram<RGB<double> >::Histogram(int size);

template<>
void	Histogram<RGB<double> >::add(const RGB<double>& p);

template<>
QPixmap	*Histogram<RGB<double> >::pixmap(int width, int height) const;

} // namespace snowgui

#endif /* _Histogram_h */
