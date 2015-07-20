/*
 * LevelMaskExtractor.cpp -- extract an level mask
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroAdapter.h>
#include <AstroDebug.h>

using namespace astro::image;

namespace astro {
namespace adapter {

#define	levelmaskextractor_typed(image, pixel)				\
	{								\
		Image<pixel>	*imagep					\
			= dynamic_cast<Image<pixel> *>(&*image);	\
		if (NULL != imagep) {					\
			LevelMaskAdapter<pixel>	lma(*imagep, _level);	\
			return ImagePtr(new Image<unsigned char>(lma));	\
		}							\
	}


ImagePtr	LevelMaskExtractor::operator()(const ImagePtr image) const {
	levelmaskextractor_typed(image, unsigned char);
	levelmaskextractor_typed(image, unsigned short);
	levelmaskextractor_typed(image, unsigned int);
	levelmaskextractor_typed(image, unsigned long);
	levelmaskextractor_typed(image, float);
	levelmaskextractor_typed(image, double);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "level mask extraction failed, "
		"inappropriate pixel type");
	throw std::runtime_error("cannot extract mask for this pixel type");
}

} // namespace image
} // namespace astro
