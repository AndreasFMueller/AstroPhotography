/*
 * ImageObjectDirectory.h -- directory containing images
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ImageObjectDirectory_h
#define _ImageObjectDirectory_h

#include <string>
#include <list>
#include <image.hh>
#include <AstroImage.h>
#include <ImageDirectory.h>

namespace Astro {

/**
 * \brief Server diretory containing images
 *
 * The ImageObjectDirectory is a singleton where image files are stored.
 * Images are identified by a string id, which can be any valid file
 * name. 
 */
class ImageObjectDirectory : public astro::image::ImageDirectory {
public:
	ImageObjectDirectory() { }
	Image_ptr	getImage(const std::string& filename);
};

} // namespace Astro

#endif /* _ImageObjectDirectory_h */
