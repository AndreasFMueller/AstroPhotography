/*
 * ColorBalance.cpp
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroAdapter.h>

namespace astro {
namespace adapter {

#if 0
//////////////////////////////////////////////////////////////////////
// Color balancing adapter
//////////////////////////////////////////////////////////////////////
template<typename T>
class ColorBalanceAdapter : public ConstImageAdapter<RGB<T> > {
	const ConstImageAdapter<RGB<T> >&	_image;
	RGB<T>	_intercept;
	RGB<T>	_slope;
public:
	ColorBalanceAdapter(const ConstImageAdapter<RGB<T> >& image)
		: ConstImageAdapter<RGB<T> >(image.getSize()), _image(image) {
		RGB<T>	sum;
		RGB<T>	sum2;
		int	w = image.getSize().width();
		int	h = image.getSize().height();
		for (int x = 0; x < w; x++) {
			for (int y = 0; y < h; y++) {
				RGB<T>	v = image.pixel(x, y);
				sum = sum + v;
				sum2 = sum2 + v * v;
			}
		}
		double	n = 1. / (w * h);
		RGB<T>	mean = sum * n;
		RGB<T>	variance = (sum2 * n) - (mean * mean);
		RGB<T>	stddev(sqrt(variance.R), sqrt(variance.G),
				sqrt(variance.B));
		RGB<T>	E = mean / stddev;
		T	EX = mean.R, stddevX = stddev.R;
		if (E.G > E.R) {
			EX = mean.G, stddevX = stddev.G;
		}
		if (E.B > E.G) {
			EX = mean.B, stddevX = stddev.B;
		}
		_slope = RGB<T>(stddevX / stddev.R, stddevX / stddev.G,
				stddevX / stddev.B);
		_intercept = RGB<T>(EX - _slope.R * mean.R,
					EX - _slope.G * mean.G,
					EX - _slope.B * mean.B);
	}
	virtual RGB<T>	pixel(int x, int y) const {
		return _image.pixel(x, y) * _slope + _intercept;
	}
};

template<typename T>
void	colorbalance(ImageAdapter<RGB<T> >& image) {
	ColorBalanceAdapter<T>	adapter(image);
	int	w = image.getSize().width();
	int	h = image.getSize().height();
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			image.writablepixel(x, y) = adapter.pixel(x, y);
		}
	}
}
#endif

#define	do_colorbalance(image, Pixel)					\
	{								\
		Image<RGB<Pixel> >	*imagep				\
			= dynamic_cast<Image<RGB<Pixel> >*>(&*image);	\
		if (NULL != imagep) {					\
			colorbalance(*imagep);				\
			return;						\
		}							\
	}

void	colorbalance(ImagePtr image) {
	do_colorbalance(image, float);
	do_colorbalance(image, double);
	throw std::runtime_error("colorbalance only available for float pixels");
}

} // namespace adapter
} // namespace astro
