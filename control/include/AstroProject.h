/*
 * AstroProject.h -- project management and data archiving
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroProject_h
#define _AstroProject_h

#include <AstroImage.h>
#include <AstroPersistence.h>

namespace astro {
namespace project {

/**
 * \brief An object containing anything but the image itself
 *
 * The ImageServer can find ImageEnvelope objects, but it can also be used
 * to request the image
 */
class ImageEnvelope {
	long	id;
	astro::image::ImageMetadata	metadata;
public:
	ImageEnvelope(const astro::image::ImagePtr image);
	std::string	cameraname() const;
	const astro::image::Metavalue&	getMetadata(const std::string& keyword) const;
};

/**
 * \brief A server for images
 */
class ImageServer {
public:
	astro::image::ImagePtr	getImage(long id);
	ImageEnvelope	getEnvelope(long id);
	long	save(astro::image::ImagePtr image);
};


} // namespace project
} // namespace astro

#endif /* _AstroProject_h */
