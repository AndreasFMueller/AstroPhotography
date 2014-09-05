/*
 * ImageEnvelope.cpp -- container for the metadata of images
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroProject.h>
#include <AstroIO.h>

using namespace astro::image;
using namespace astro::io;

namespace astro {
namespace project {

/**
 * \brief Construct metadata from an image
 */
ImageEnvelope::ImageEnvelope(const ImagePtr image) {
	copy_metadata(*image, metadata, FITSKeywords::names());
}

/**
 * \brief access the meta data
 */
const Metavalue&	ImageEnvelope::getMetadata(const std::string& keyword) const {
	return metadata.getMetadata(keyword);
}

/**
 * \brief get the name of the camera
 */
std::string	ImageEnvelope::cameraname() const {
	return (std::string)getMetadata("CAMERA");
}

} // namespace project
} // namespace astro
