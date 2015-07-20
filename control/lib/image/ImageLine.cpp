/*
 * ImageLine.cpp -- classes to access rows and columns of an image
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroImage.h>

namespace astro {
namespace image {

/**
 * \brief get an iterator pointing to the beginning of a line
 */
ImageIteratorBase       ImageLine::begin() const {
	return ImageIteratorBase(firstoffset, lastoffset, firstoffset, stride);
}

/**
 * \brief get an iterator pointing to the end of a line
 */
ImageIteratorBase       ImageLine::end() const {
	return ImageIteratorBase(firstoffset, lastoffset, -1, stride);
}

} // namespace image
} // namespace astro
