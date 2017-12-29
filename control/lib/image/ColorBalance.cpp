/*
 * ColorBalance.cpp
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroAdapter.h>

namespace astro {
namespace adapter {

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
