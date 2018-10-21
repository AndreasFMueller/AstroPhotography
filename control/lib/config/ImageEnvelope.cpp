/*
 * ImageEnvelope.cpp -- container for the metadata of images
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroProject.h>
#include <AstroIO.h>
#include <sstream>

using namespace astro::image;
using namespace astro::io;

namespace astro {
namespace project {

/**
 * \brief Construct empty envelope
 */
ImageEnvelope::ImageEnvelope(long id) : _id(id) {
	_created = time(NULL);
	_observation = 0;
	_exposuretime = 0.;
	_temperature = 0.;
	_focus = 0;
}

/**
 * \brief Construct metadata from an image
 */
ImageEnvelope::ImageEnvelope(const ImagePtr image) : _size(image->size()) {
	_created = time(NULL);
	_observation = 0;
	_exposuretime = 0.;
	_temperature = 0.;
	_focus = 0;
	copy_metadata(*image, metadata);
}

/**
 * \brief access the meta data
 */
const Metavalue&	ImageEnvelope::getMetadata(const std::string& keyword) const {
	return metadata.getMetadata(keyword);
}

std::string	ImageEnvelope::toString() const {
	std::stringstream	out;
	out << "id = " << _id << ", size = " << _size.toString() << std::endl;
	ImageMetadata::const_iterator	mi;
	for (mi = metadata.begin(); mi != metadata.end(); mi++) {
		out << mi->second.toString() << std::endl;
	}
	return out.str();
}

} // namespace project
} // namespace astro
