/*
 * luminancemapping.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroTonemapping.h>
#include <AstroDebug.h>

namespace astro {
namespace adapter {

#define do_luminancemapping(Pixel, image, luminancefunctionptr)		\
{									\
	Image<Pixel>	*img = dynamic_cast<Image<Pixel >*>(&*image);	\
	if (NULL != img) {						\
		LuminanceFunctionPtrAdapter<Pixel>	lma(*img,	\
			luminancefunctionptr);				\
		return ImagePtr(new Image<Pixel>(lma));			\
	}								\
}


/**
 * \brief Perform luminancen mapping for a given luminance function
 *
 * \param image			the image to map
 * \param luminancefunctionptr	the luminance mapping function to use
 */
ImagePtr	luminancemapping(ImagePtr image,
			LuminanceFunctionPtr luminancefunctionptr) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "do luminance mapping in %s image, "
		"function %s", image->info().c_str(),
		luminancefunctionptr->info().c_str());
	do_luminancemapping(unsigned char, image, luminancefunctionptr)
	do_luminancemapping(unsigned short, image, luminancefunctionptr)
	do_luminancemapping(unsigned int, image, luminancefunctionptr)
	do_luminancemapping(unsigned long, image, luminancefunctionptr)
	do_luminancemapping(float, image, luminancefunctionptr)
	do_luminancemapping(double, image, luminancefunctionptr)
	do_luminancemapping(RGB<unsigned char>, image, luminancefunctionptr)
	do_luminancemapping(RGB<unsigned short>, image, luminancefunctionptr)
	do_luminancemapping(RGB<unsigned int>, image, luminancefunctionptr)
	do_luminancemapping(RGB<unsigned long>, image, luminancefunctionptr)
	do_luminancemapping(RGB<float>, image, luminancefunctionptr)
	do_luminancemapping(RGB<double>, image, luminancefunctionptr)
	std::string	msg = stringprintf("cannot do luminance mapping for "
		"%s image", image->info().c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

} // namespace adapter
} // namespace astro
