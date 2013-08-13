/*
 * AstroStacking.h -- primitive stacking function
 * 
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroStacking_h
#define _AstroStacking_h

#include <AstroImage.h>

namespace astro {
namespace image {
namespace stacking {

class Stacker {
public:
	ImagePtr	operator()(ImageSequence images);
};

} // namespace stacking
} // namespace image
} // namespace astro

#endif /* _AstroStacking_h */
