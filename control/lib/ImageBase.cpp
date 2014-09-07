/*
 * ImageBase.cpp -- implementation of base image class methods
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroImage.h>
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <typeinfo>

namespace astro {
namespace image {

/**
 * \brief Construct an image base from size parameters
 */
ImageBase::ImageBase(unsigned int w, unsigned int h) : frame(w, h) {
}

ImageBase::ImageBase(const ImageSize& _size) : frame(_size) {
}

ImageBase::ImageBase(const ImageRectangle& _frame) : frame(_frame) {
}

ImageBase::ImageBase(const ImageBase& other) : frame(other.frame) {
	mosaic = other.mosaic;
}

/**
 * \brief Compare two images
 *
 * Two images are considered equal if the have identical size.
 */
bool	ImageBase::operator==(const ImageBase& other) const {
	return (frame == other.frame);
}

/**
 * \brief Compute the pixel offset into an Image based on coordinates
 */
unsigned int     ImageBase::pixeloffset(unsigned int x, unsigned int y) const {
	return frame.size().offset(x, y);
}

/**
 * \brief Compute the pixel offset into an Image based on an ImagePoint
 */
unsigned int     ImageBase::pixeloffset(const ImagePoint& p) const {
	return frame.size().offset(p);
}

unsigned int	ImageBase::bytesPerPixel() const {
	return this->bitsPerPixel() / 8;
}

unsigned int	ImageBase::bytesPerPlane() const {
	return bytesPerPixel() / planes();
}

unsigned int	ImageBase::bitsPerPlane() const {
	return bitsPerPixel() / planes();
}

static std::string	mosaic_key("BAYER");

/**
 * \brief set mosaic type
 *
 * This method ensures that the metadata map and the mosaic type
 * are consistent.
 */
void	ImageBase::setMosaicType(MosaicType::mosaic_type _mosaic) {
	mosaic.setMosaicType(_mosaic);

	// remove the key
	ImageMetadata::iterator	i = metadata.find(mosaic_key);
	if (i != metadata.end()) {
		metadata.erase(i);
	}

	// compute the new key value
	std::string	value;
	switch (_mosaic) {
	case MosaicType::BAYER_RGGB:
		value = "RGGB";
		break;
	case MosaicType::BAYER_GRBG:
		value = "GRBG";
		break;
	case MosaicType::BAYER_GBRG:
		value = "GBRG";
		break;
	case MosaicType::BAYER_BGGR:
		value = "BGGR";
		break;
	default:
		// don't store NONE or any other unknown types
		break;
	}

	// if the new value is nonzero, add it
	if (value.size() > 0) {
		Metavalue	mv("BAYER", value, std::string("Bayer Color Matrix"));
		std::pair<std::string, Metavalue>	p(mosaic_key, mv);
		metadata.insert(metadata.begin(), p);
	}
}

/**
 * \brief Set mosaic type from name
 *
 * This method ensures that only valid mosaic type names are used and
 * that the mosaic_type member variable is consistently set.
 * \param mosaic_name	string representation of color mosaic
 */
void	ImageBase::setMosaicType(const std::string& mosaic_name) {
	if (mosaic_name == "NONE") {
		setMosaicType(MosaicType::NONE);
		return;
	}
	if (mosaic_name == "RGGB") {
		setMosaicType(MosaicType::BAYER_RGGB);
		return;
	}
	if (mosaic_name == "GRBG") {
		setMosaicType(MosaicType::BAYER_GRBG);
		return;
	}
	if (mosaic_name == "GBRG") {
		setMosaicType(MosaicType::BAYER_GBRG);
		return;
	}
	if (mosaic_name == "BGGR") {
		setMosaicType(MosaicType::BAYER_BGGR);
		return;
	}
	std::string	msg = stringprintf("unknown mosaic name: %s",
		mosaic_name.c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

/**
 * \brief Find out whether a given Metadata value is set
 *
 * \param name	 name of the metadata element
 */
bool	ImageBase::hasMetadata(const std::string& name) const {
	return metadata.hasMetadata(name);
}

/**
 * \brief Retrieve metadata
 */
Metavalue	ImageBase::getMetadata(const std::string& name) const {
	return metadata.getMetadata(name);
}

/**
 * \brief Remove the metadata of a given type
 */
void	ImageBase::removeMetadata(const std::string& name) {
	metadata.remove(name);
}

/**
 * \brief Update metadata
 */
void	ImageBase::setMetadata(const Metavalue& mv) {
	metadata.setMetadata(mv);
}

std::ostream&	operator<<(std::ostream& out, const ImageBase& image) {
	out << "size: " << image.frame.size().toString() << std::endl;
	ImageMetadata::const_iterator	i;
	for (i = image.metadata.begin(); i != image.metadata.end(); i++) {
		out << i->first << ": ";
		out << i->second.getValue() << " / ";
		out << i->second.getComment() << std::endl;
	}
	return out;
}

ImageMetadata::const_iterator	ImageBase::begin() const {
	return metadata.begin();
}

ImageMetadata::const_iterator	ImageBase::end() const {
	return metadata.end();
}

#if 0
/**
 * \brief retrieve the image type
 */
static std::string      get_typename(const ImageBase *image) {
	try {
                return std::string(typeid(*image).name());
        } catch (std::bad_typeid& x) {
                return stringprintf("(unknown Image type[%s])", x.what());
        }
}

/**
 * \brief Get the name of the image (mainly for debugging)
 */
std::string     ImageBase::type_name() const {
        return get_typename(this);
}
#endif

} // namespace image
} // namespace astro
