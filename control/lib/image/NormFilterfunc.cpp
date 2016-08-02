/*
 * NormFilterfunc.cpp -- filterfunctions that compute the norm of an image
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroFilterfunc.h>
#include <AstroAdapter.h>
#include <AstroFilter.h>

using namespace astro::image;

namespace astro {
namespace image {
namespace filter {

#define filterl1(image, Pixel)						\
{									\
	Image<Pixel >	*imagep						\
		= dynamic_cast<Image<Pixel > *>(&*image);		\
	if (NULL != imagep) {						\
		adapter::LuminanceAdapter<Pixel, double> la(*imagep);	\
		Sum<double>	sum;					\
		return sum.filter(la);					\
	}								\
}

#define filterl2(image, Pixel) 						\
{									\
	Image<Pixel >	*imagep						\
		= dynamic_cast<Image<Pixel > *>(&*image);		\
	if (NULL != imagep) {						\
		adapter::LuminanceAdapter<Pixel, double> la(*imagep);	\
		adapter::SquareAdapter<double>	sa(la);			\
		Sum<double>	sum;					\
		return sqrt(sum.filter(sa));				\
	}								\
}

#define filterlp(image, Pixel, p)					\
{									\
	Image<Pixel >	*imagep						\
		= dynamic_cast<Image<Pixel > *>(&*image);		\
	if (NULL != imagep) {						\
		adapter::LuminanceAdapter<Pixel, double> la(*imagep);	\
		adapter::PowerAdapter<double>	sa(la, p);		\
		Sum<double>	sum;					\
		return pow(sum.filter(sa), 1/p);			\
	}								\
}

double   l1norm(const ImagePtr& image) {
	filterl1(image, unsigned char)
	filterl1(image, unsigned short)
	filterl1(image, unsigned int)
	filterl1(image, float)
	filterl1(image, double)
	filterl1(image, RGB<unsigned char>)
	filterl1(image, RGB<unsigned short>)
	filterl1(image, RGB<unsigned int>)
	filterl1(image, RGB<float>)
	filterl1(image, RGB<double>)
	return 0;
}

double   l2norm(const ImagePtr& image) {
	filterl2(image, unsigned char)
	filterl2(image, unsigned short)
	filterl2(image, unsigned int)
	filterl2(image, float)
	filterl2(image, double)
	filterl2(image, RGB<unsigned char>)
	filterl2(image, RGB<unsigned short>)
	filterl2(image, RGB<unsigned int>)
	filterl2(image, RGB<float>)
	filterl2(image, RGB<double>)
	return 0;
}

double   lpnorm(const ImagePtr& image, double p) {
	filterlp(image, unsigned char, p)
	filterlp(image, unsigned short, p)
	filterlp(image, unsigned int, p)
	filterlp(image, float, p)
	filterlp(image, double, p)
	filterlp(image, RGB<unsigned char>, p)
	filterlp(image, RGB<unsigned short>, p)
	filterlp(image, RGB<unsigned int>, p)
	filterlp(image, RGB<float>, p)
	filterlp(image, RGB<double>, p)
	return 0;
}

} // namespace filter
} // namespace image
} // namespace astro
