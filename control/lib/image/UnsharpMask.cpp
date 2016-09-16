/*
 * UnsharpMask.cpp
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroAdapter.h>

namespace astro {
namespace adapter {

UnsharpMaskBase::UnsharpMaskBase() {
	radius(2);
}

double	UnsharpMaskBase::w(int x, int y) const {
	double	s = hypot(x, y) / _radius;
	return 1 - s * s;
}

void	UnsharpMaskBase::radius(double r) {
	_radius = r;
	_top = ceil(r);
	_weight = 0;
	int	counter = 0;
	for (int x = -_top; x <= _top; x++) {
		for (int y = -_top; y <= _top; y++) {
			double	s = w(x, y);
			if (s > 0) {
				_weight += s;
				counter++;
			}
		}
	}
	_weight = 1./_weight;
}

template<typename T>
ImagePtr	unsharp(const ConstImageAdapter<T>& image,
			double radius, double amount) {
	TilingAdapter<T>	tiling(image);
	UnsharpMaskingAdapter<T>	masking(tiling);
	masking.radius(radius);
	masking.amount(amount);
	return ImagePtr(new Image<T>(masking));
}

#define	do_unsharp(image, Pixel)					\
	{								\
		Image<Pixel>	*imagep					\
			= dynamic_cast<Image<Pixel >*>(&*image);	\
		if (NULL != imagep) {					\
			return unsharp(*imagep, radius, amount);	\
		}							\
	}

ImagePtr	unsharp(ImagePtr image, double radius, double amount) {
	do_unsharp(image, float);
	do_unsharp(image, double);
	throw std::runtime_error("cannot unsharp mask this image type");
}

} // namespace adapter
} // namespace astro
