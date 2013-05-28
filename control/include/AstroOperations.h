/*
 * AstroOperations.h -- templates for operators to perform image operations
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroOperations_h
#define _AstroOperations_h

namespace astro {
namespace image {

ImagePtr	operator+(const ImagePtr& a, const ImagePtr& b);
ImagePtr	operator-(const ImagePtr& a, const ImagePtr& b);
ImagePtr	operator*(const ImagePtr& a, const ImagePtr& b);
ImagePtr	operator/(const ImagePtr& a, const ImagePtr& b);

ImagePtr	average(const std::vector<ImagePtr>& images);

} // namespace image
} // namespace astro

#endif /* _AstroOperations_h */
