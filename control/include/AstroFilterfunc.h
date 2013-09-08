/*
 * AstroFilterfunc.h -- filters to apply to images 
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroFilterfunc_h
#define _AstroFilterfunc_h

#include <AstroImage.h>
#include <AstroMask.h>

namespace astro {
namespace image {
namespace filter {

extern double	countnans(const ImagePtr& image);
extern double	countnansrel(const ImagePtr& image);

extern double	max(const ImagePtr& image);
extern double	max_luminance(const ImagePtr& image);
extern double	max_RGB(const ImagePtr& image);
extern double	maxrel(const ImagePtr& image);

extern double	min(const ImagePtr& image);
extern double	min_luminance(const ImagePtr& image);
extern double	min_RGB(const ImagePtr& image);
extern double	minrel(const ImagePtr& image);

extern double	mean(const ImagePtr& image);
extern double	meanrel(const ImagePtr& image);

extern double	median(const ImagePtr& image);

extern int	bytespervalue(const ImagePtr& image);
extern int	bytesperpixel(const ImagePtr& image);
extern int	planes(const ImagePtr& image);

extern double	focusFOM(const ImagePtr& image, const bool diagonal = false);
extern double	focusFWHM(const ImagePtr& image, const ImagePoint& where,
			unsigned int r);

extern void	mask(MaskingFunction& maskingfunction, ImagePtr image);

extern double	rawvalue(const ImagePtr& image, const ImagePoint& point);

extern bool	saturated(const ImagePtr& image, const ImageRectangle& rect);


} // namespace filter
} // namespace image
} // namespace astro

#endif /* _AstroFilterfunc_h */
