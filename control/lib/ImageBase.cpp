/*
 * ImageBase.cpp
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroImage.h>
#include <Format.h>
#include <debug.h>

namespace astro {
namespace image {

/**
 * \brief Construct an image base from size parameters
 */
ImageBase::ImageBase(unsigned int w, unsigned int h) : size(w, h) {
	mosaic = NONE;
}

ImageBase::ImageBase(const ImageSize& _size) : size(_size) {
	mosaic = NONE;
}

ImageBase::ImageBase(const ImageRectangle& frame) : size(frame.size) {
	mosaic = NONE;
}

ImageBase::ImageBase(const ImageBase& other) : size(other.size) {
	mosaic = other.mosaic;
}

/**
 * \brief Compare two images
 *
 * Two images are considered equal if the have identical size.
 */
bool	ImageBase::operator==(const ImageBase& other) const {
	return (size == other.size);
}

/**
 * \brief Compute the pixel offset into an Image based on coordinates
 */
unsigned int     ImageBase::pixeloffset(unsigned int x, unsigned int y) const {
	return size.offset(x, y);
}

/**
 * \brief Compute the pixel offset into an Image based on an ImagePoint
 */
unsigned int     ImageBase::pixeloffset(const ImagePoint& p) const {
	return size.offset(p);
}

bool	ImageBase::isR(unsigned int x, unsigned int y) const {
	if (mosaic == NONE) {
		return false;
	}
	return (((y & 0x1) << 1) | (x & 0x1)) == (mosaic & 0x3);
}

bool	ImageBase::isB(unsigned int x, unsigned int y) const {
	if (mosaic == NONE) {
		return false;
	}
	// this means that the mod 2 remainder of both x and y have to
	// be different from the ones in the mosaic constant. The XOR
	// with 0x3 inverts the coordinates so that we can nevertheless
	// do an equality comparison
	return (0x3 ^ (((y & 0x1) << 1) | (x & 0x1))) == (mosaic & 0x3);
}

bool	ImageBase::isG(unsigned int x, unsigned int y) const {
	return (isGr(x, y) | isGb(x, y));
	
}

bool	ImageBase::isGr(unsigned int x, unsigned int y) const {
	if (mosaic == NONE) {
		return false;
	}
	return (0x1 ^ (((y & 0x1) << 1) | (x & 0x1))) == (mosaic & 0x3);
}

bool	ImageBase::isGb(unsigned int x, unsigned int y) const {
	if (mosaic == NONE) {
		return false;
	}
	return (0x2 ^ (((y & 0x1) << 1) | (x & 0x1))) == (mosaic & 0x3);
}

unsigned int	ImageBase::bytesPerPixel() const {
	return this->bitsPerPixel() / 8;
}

ImageBase::mosaic_type	ImageBase::getMosaicType() const {
	return mosaic;
}

static std::string	mosaic_key("BAYER");

/**
 * \brief set mosaic type
 *
 * This method ensures that the metadata map and the mosaic type
 * are consistent.
 */
void	ImageBase::setMosaicType(ImageBase::mosaic_type _mosaic) {
	mosaic = _mosaic;

	// remove the key
	ImageMetadata::iterator	i = metadata.find(mosaic_key);
	if (i != metadata.end()) {
		metadata.erase(i);
	}

	// compute the new key value
	std::string	value;
	switch (mosaic) {
	case ImageBase::BAYER_RGGB:
		value = "RGGB";
		break;
	case ImageBase::BAYER_GRBG:
		value = "GRBG";
		break;
	case ImageBase::BAYER_GBRG:
		value = "GBRG";
		break;
	case ImageBase::BAYER_BGGR:
		value = "BGGR";
		break;
	default:
		// don't store NONE or any other unknown types
		break;
	}

	// if the new value is nonzero, add it
	if (value.size() > 0) {
		Metavalue	mv(value, std::string("Bayer Color Matrix"));
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
		setMosaicType(ImageBase::NONE);
		return;
	}
	if (mosaic_name == "RGGB") {
		setMosaicType(ImageBase::BAYER_RGGB);
		return;
	}
	if (mosaic_name == "GRBG") {
		setMosaicType(ImageBase::BAYER_GRBG);
		return;
	}
	if (mosaic_name == "GBRG") {
		setMosaicType(ImageBase::BAYER_GBRG);
		return;
	}
	if (mosaic_name == "BGGR") {
		setMosaicType(ImageBase::BAYER_BGGR);
		return;
	}
	std::string	msg = stringprintf("unknown mosaic name: %s",
		mosaic_name.c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

/**
 * \brief
 */
bool	ImageBase::isMosaic() const {
	return mosaic != NONE;
}

/**
 * \brief Find out whether a given Metadata value is set
 *
 * \param name	 name of the metadata element
 */
bool	ImageBase::hasMetadata(const std::string& name) const {
	return (metadata.find(name) != metadata.end());
}

/**
 * \brief Retrieve metadata
 */
Metavalue	ImageBase::getMetadata(const std::string& name) const {
	if (hasMetadata(name)) {
		return metadata.find(name)->second;
	}
	std::string	msg = stringprintf("image has no '%s' metadata",
		name.c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

/**
 * \brief Update metadata
 */
void	ImageBase::setMetadata(const std::string& name, const Metavalue& mv) {
	// XXX ensure that values that are managed by the image functions
	//     cannot be set through this interface (the ignored function
	//     in Fits.cpp has this functionality), unfortuntately, it's in
	//     wrong file...
	
	if (hasMetadata(name)) {
		//metadata[name] = mv;
	} else {
		metadata.insert(make_pair(name, mv));
	}
}

std::ostream&	operator<<(std::ostream& out, const ImageBase& image) {
	out << "size: " << image.size.width << " x "
                        << image.size.height <<std::endl;
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

} // namespace image
} // namespace astro
