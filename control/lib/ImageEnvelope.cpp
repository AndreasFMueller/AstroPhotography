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
ImageEnvelope::ImageEnvelope(const ImagePtr image) : _size(image->size()) {
	copy_metadata(*image, metadata);
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

float	ImageEnvelope::exposuretime() const {
	double	t = getMetadata("EXPTIME");
	return t;
}

float	ImageEnvelope::temperature() const {
	double	t = getMetadata("CCD-TEMP");
	return t;
};

ImageSpec::category_t	ImageEnvelope::category() const {
	std::string	purpose = getMetadata("PURPOSE");
	if (purpose == "dark") {
		return ImageSpec::dark;
	}
	if (purpose == "flat") {
		return ImageSpec::flat;
	}
	if (purpose == "light") {
		return ImageSpec::light;
	}
	throw std::runtime_error("internal error: unknown purpose");
}

} // namespace project
} // namespace astro
