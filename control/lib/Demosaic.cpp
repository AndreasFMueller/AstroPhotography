/*
 * Demosaic.cpp -- demosaic an image with the same type
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroDemosaic.h>
#include <debug.h>
#include <stdexcept>

namespace astro {
namespace image {

#define	demosaic_bilinear_for(image, P)					\
{									\
	Image<P>	*timage = dynamic_cast<Image<P> *>(&*image);	\
	if (NULL != timage) {						\
		DemosaicBilinear<P>	demosaicer;			\
		return ImagePtr(demosaicer(*timage));			\
	}								\
}

ImagePtr	demosaic_bilinear(const ImagePtr& image) {
	demosaic_bilinear_for(image, unsigned char);
	demosaic_bilinear_for(image, unsigned short);
	demosaic_bilinear_for(image, unsigned int);
	demosaic_bilinear_for(image, unsigned long);
	demosaic_bilinear_for(image, float);
	demosaic_bilinear_for(image, double);
	std::string	msg("unknown pixel type: cannot demosaic");
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

} // namespace image
} // namespace astro
