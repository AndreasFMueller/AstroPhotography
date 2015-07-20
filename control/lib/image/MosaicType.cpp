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
 * \brief Set mosaic type from name
 *
 * This method ensures that only valid mosaic type names are used and
 * that the mosaic_type member variable is consistently set.
 * \param mosaic_name	string representation of color mosaic
 */
void	MosaicType::setMosaicType(const std::string& mosaic_name) {
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

void	MosaicType::setMosaicType(mosaic_type _mosaic) {
	mosaic = _mosaic;
}

/**
 * \brief 
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
	unsigned int	redx =  mosaic       & 0x1;
	unsigned int	redy = (mosaic >> 1) & 0x1;
	return ImagePoint(redx, redy);
}

ImagePoint	MosaicType::blue() const {
	ImagePoint	r = red();
	unsigned int	bluex = 0x1 ^ r.x();
	unsigned int	bluey = 0x1 ^ r.y();
	return ImagePoint(bluex, bluey);
}

ImagePoint	MosaicType::greenr() const {
	ImagePoint	r = red();
	unsigned int	bluex = 0x1 ^ r.x();
	return ImagePoint(bluex, r.y());
}

ImagePoint	MosaicType::greenb() const {
	ImagePoint	r = red();
	unsigned int	bluey = 0x1 ^ r.y();
	return ImagePoint(r.x(), bluey);
}

} // namespace image
} // namespace astro
