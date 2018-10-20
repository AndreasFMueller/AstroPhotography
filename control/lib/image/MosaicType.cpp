/*
 * MosaicType.cpp -- Mosaic type implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroImage.h>
#include <AstroFormat.h>

namespace astro {
namespace image {

/**
 * \brief conversion of mosaic type from string
 */
MosaicType::mosaic_type	MosaicType::string2type(const std::string& mosaic_name) {
	if (mosaic_name == "RGGB") {
		return BAYER_RGGB;
	}
	if (mosaic_name == "GRBG") {
		return BAYER_GRBG;
	}
	if (mosaic_name == "GBRG") {
		return BAYER_GBRG;
	}
	if (mosaic_name == "BGGR") {
		return BAYER_BGGR;
	}
	if (mosaic_name == "NONE") {
		return NONE;
	}
	std::string	msg = stringprintf("unknown mosaic name: %s",
		mosaic_name.c_str());
	debug(LOG_WARNING, DEBUG_LOG, 0, "%s", msg.c_str());
	return NONE;
}

/**
 * \brief conversion of mosaic type from string
 */
std::string	MosaicType::type2string(mosaic_type t) {
	switch (t) {
	case BAYER_RGGB: return std::string("RGGB");
	case BAYER_GRBG: return std::string("GRBG");
	case BAYER_GBRG: return std::string("GBRG");
	case BAYER_BGGR: return std::string("BGGR");
	default:
		break;
	}
	return std::string("NONE");
}

/**
 * \brief Shift a mosaic for an image point offset
 *
 * \param mosaic	the unshifted mosic
 * \param offset	the offset of subframe
 */
MosaicType::mosaic_type	MosaicType::shift(MosaicType::mosaic_type mosaic,
			const ImagePoint& offset) {
	// if there is no mosaic, there is no need to change anything
	if (mosaic == NONE) {
		return NONE;
	}
	// only the last bit of the offset actually is important when
	// performing the shift
	unsigned char	shift = ((offset.x() & 0x1) |
					((offset.y() & 0x1) << 1));
	mosaic_type	newmosaic = (mosaic_type)((int)mosaic ^ shift);
	return newmosaic;
}

MosaicType::mosaic_type	MosaicType::vflip(MosaicType::mosaic_type t) {
	switch (t) {
	case NONE:		return NONE;
	case BAYER_RGGB:	return BAYER_GBRG;
	case BAYER_GRBG:	return BAYER_BGGR;
	case BAYER_GBRG:	return BAYER_RGGB;
	case BAYER_BGGR:	return BAYER_GRBG;
	}
	throw std::logic_error("internal error, bad mosaic type");
}

MosaicType::mosaic_type	MosaicType::hflip(MosaicType::mosaic_type t) {
	switch (t) {
	case NONE:		return NONE;
	case BAYER_RGGB:	return BAYER_GRBG;
	case BAYER_GRBG:	return BAYER_RGGB;
	case BAYER_GBRG:	return BAYER_BGGR;
	case BAYER_BGGR:	return BAYER_GBRG;
	}
	throw std::logic_error("internal error, bad mosaic type");
}

MosaicType::mosaic_type	MosaicType::rotate(MosaicType::mosaic_type t) {
	switch (t) {
	case NONE:		return NONE;
	case BAYER_RGGB:	return BAYER_BGGR;
	case BAYER_GRBG:	return BAYER_GBRG;
	case BAYER_GBRG:	return BAYER_GRBG;
	case BAYER_BGGR:	return BAYER_RGGB;
	}
	throw std::logic_error("internal error, bad mosaic type");
}

/**
 * \brief Construct a MosaicType object from code and offset
 */
MosaicType::MosaicType(mosaic_type _mosaic, ImagePoint offset)
	: mosaic(shift(_mosaic, offset)) {
}

/**
 * \brief Construct a MosaicType object from the mosaic name
 */
MosaicType::MosaicType(const std::string& mosaic_name, ImagePoint offset)
	: mosaic(shift(string2type(mosaic_name), offset)) {
}

/**
 * \brief Set mosaic type from name
 *
 * This method ensures that only valid mosaic type names are used and
 * that the mosaic_type member variable is consistently set.
 *
 * \param mosaic_name	string representation of color mosaic
 * \param offset	the optional offset of the mosaic
 */
void	MosaicType::setMosaicType(const std::string& mosaic_name,
		ImagePoint offset) {
	setMosaicType(shift(string2type(mosaic_name), offset));
}

void	MosaicType::setMosaicType(MosaicType::mosaic_type _mosaic,
		ImagePoint offset) {
	mosaic = shift(_mosaic, offset);
}

/**
 * \brief Whether or not there is a mosaic at all
 */
bool	MosaicType::isMosaic() const {
	return mosaic != NONE;
}

bool	MosaicType::isR(unsigned int x, unsigned int y) const {
	if (mosaic == NONE) {
		return false;
	}
	return (((y & 0x1) << 1) | (x & 0x1)) == (mosaic & 0x3);
}

bool	MosaicType::isB(unsigned int x, unsigned int y) const {
	if (mosaic == NONE) {
		return false;
	}
	// this means that the mod 2 remainder of both x and y have to
	// be different from the ones in the mosaic constant. The XOR
	// with 0x3 inverts the coordinates so that we can nevertheless
	// do an equality comparison
	return (0x3 ^ (((y & 0x1) << 1) | (x & 0x1))) == (mosaic & 0x3);
}

bool	MosaicType::isG(unsigned int x, unsigned int y) const {
	return (isGr(x, y) | isGb(x, y));
	
}

bool	MosaicType::isGr(unsigned int x, unsigned int y) const {
	if (mosaic == NONE) {
		return false;
	}
	return (0x1 ^ (((y & 0x1) << 1) | (x & 0x1))) == (mosaic & 0x3);
}

bool	MosaicType::isGb(unsigned int x, unsigned int y) const {
	if (mosaic == NONE) {
		return false;
	}
	return (0x2 ^ (((y & 0x1) << 1) | (x & 0x1))) == (mosaic & 0x3);
}

ImagePoint	MosaicType::red() const {
	int	redx =  mosaic       & 0x1;
	int	redy = (mosaic >> 1) & 0x1;
	return ImagePoint(redx, redy);
}

ImagePoint	MosaicType::blue() const {
	ImagePoint	r = red();
	int	bluex = 0x1 ^ r.x();
	int	bluey = 0x1 ^ r.y();
	return ImagePoint(bluex, bluey);
}

ImagePoint	MosaicType::greenr() const {
	ImagePoint	r = red();
	int	bluex = 0x1 ^ r.x();
	return ImagePoint(bluex, r.y());
}

ImagePoint	MosaicType::greenb() const {
	ImagePoint	r = red();
	int	bluey = 0x1 ^ r.y();
	return ImagePoint(r.x(), bluey);
}

MosaicType	MosaicType::shifted(const ImagePoint& offset) const {
	return MosaicType(shift(mosaic, offset));
}

MosaicType	MosaicType::shifted(const ImageRectangle& rectangle) const {
	return shifted(rectangle.origin());
}

MosaicType	MosaicType::operator()(const ImagePoint& offset) const {
	return shifted(offset);
}

MosaicType	MosaicType::operator()(const ImageRectangle& rectangle) const {
	return shifted(rectangle);
}

MosaicType	MosaicType::vflip() const {
	return MosaicType(vflip(mosaic));
}

MosaicType	MosaicType::hflip() const {
	return MosaicType(hflip(mosaic));
}

MosaicType	MosaicType::rotate() const {
	return MosaicType(rotate(mosaic));
}

} // namespace image
} // namespace astro
