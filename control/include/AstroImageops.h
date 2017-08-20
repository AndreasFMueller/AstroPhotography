/*
 * AstroImageop.h -- Image operations
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rappeswil
 */
#ifndef _AstroImageop_h
#define _AstroImageop_h

#include <AstroImage.h>
#include <AstroDebug.h>
#include <stdexcept>
#include <AstroAdapter.h>
#include <AstroIO.h>

namespace astro {
namespace image {
namespace ops {

template<typename Pixel>
Image<Pixel>	*cut(const Image<Pixel>& source, const ImageRectangle& rect) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "cut image %s to rectangle %s",
		source.getFrame().toString().c_str(), rect.toString().c_str());
	// compute the image rectangle we want to cut out
	ImageRectangle	sourcerect = source.getFrame();
	if (!sourcerect.contains(rect)) {
		throw std::runtime_error("rectangle not contained in image");
	}
	ImagePoint	origin = rect.origin() - sourcerect.origin();
	ImageSize	size = rect.size();
	ImageRectangle	targetrect(origin, size);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adapter rectangle is %s",
		targetrect.toString().c_str());

	// cut out the image data
	adapter::WindowAdapter<Pixel>	wa(source, targetrect);
	Image<Pixel>	*result = new Image<Pixel>(wa);
	result->setOrigin(rect.origin());
	
	// copy the metadata
	ImageMetadata::const_iterator	i;
	for (i = source.begin(); i != source.end(); i++) {
		result->setMetadata(i->second);
	}

	// fix the subframe data
	result->setMetadata(
                io::FITSKeywords::meta(std::string("XORGSUBF"),
                        (long)rect.origin().x()));
	result->setMetadata(
                io::FITSKeywords::meta(std::string("YORGSUBF"),
                        (long)rect.origin().y()));

	// that's the copy
	debug(LOG_DEBUG, DEBUG_LOG, 0, "result image: %s",
		result->getFrame().toString().c_str());
	return result;
}

ImagePtr	cut(ImagePtr source, const ImageRectangle& rect);

ImagePtr	duplicate(ImagePtr image);

} // namespace ops
} // namespace image
} // namespace astro


#endif /* _AstroImageop_h */
