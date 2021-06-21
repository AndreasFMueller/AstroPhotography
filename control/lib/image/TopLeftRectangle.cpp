/*
 * TopLeftRectangle.cpp -- a derived class to more easily handle images where
 *                         the origin is in the top left corner 
 *
 * (c) 2021 Prof Dr Andreas Müller, OST Ostschweizer Fachhochschule
 */
#include <AstroImage.h>

namespace astro {
namespace image {

/**
 * \brief Default construct the topleftrectangle
 *
 * This type of topleft rectangle is pretty useless but it is consistent
 */
TopLeftRectangle::TopLeftRectangle() {
}

/**
 * \brief Construct a top left rectangle from origin, size and bounds rectangle
 *
 * \param topleft	the top left corner of the image rectangle
 * \param size		the size of the rectangle 
 * \param within	the size of the rectangle within which the top left
 *			rectangle lives
 */
TopLeftRectangle::TopLeftRectangle(const ImagePoint& topleft,
	const ImageSize& size, const ImageSize& within)
	: ImageRectangle(ImagePoint(topleft.x(),
		within.height() - topleft.y() - size.height()), size),
	  _within(within) {
	check();
}

/**
 * \brief Construct a top left rectangle from a rectangle and bounds
 *
 * \param rectangle	the rectangle
 * \param within	the bounds rectangle
 */
TopLeftRectangle::TopLeftRectangle(const ImageRectangle& rectangle,
	const ImageSize& within) : ImageRectangle(rectangle), _within(within) {
	check();
}

/**
 * \brief Verify consistency of the rectangle
 */
void	TopLeftRectangle::check() const {
	if (!fits(within())) {
		std::string	msg = stringprintf("%s does not fit",
			this->toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::range_error(msg);
	}
}

/**
 * \brief Convert the rectangle to a string
 */
std::string	TopLeftRectangle::toString() const {
	return stringprintf("%s inside %s, toplevel=%s",
		ImageRectangle::toString().c_str(),
		within().toString().c_str(),
		topleft().toString().c_str());
}

/**
 * \brief Compute the top left corner
 */
ImagePoint	TopLeftRectangle::topleft() const {
	return ImagePoint(origin().x(),
		within().height() - size().height() - origin().y());
}

/**
 * \brief Get a subrectangle of a TopLeftRectangle
 *
 * \param rect		rectangle
 */
TopLeftRectangle	TopLeftRectangle::subrectangle(
				const ImageRectangle& rect) const {
	return TopLeftRectangle(ImageRectangle::subrectangle(rect), within());
}

/**
 * \brief Bin a top left rectangle
 *
 * \param rectangle	The rectangle to bin
 * \param bin		the binning mode
 */
TopLeftRectangle	operator/(const TopLeftRectangle& rectangle,
				const Binning& bin) {
	return TopLeftRectangle((ImageRectangle)rectangle / bin,
		rectangle.within() / bin);
}

/**
 * \brief Unbin a top left rectangle
 *
 * \param rectangle	The rectangle to unbin
 * \param bin		the binning mode
 */
TopLeftRectangle	operator*(const TopLeftRectangle& rectangle,
				const Binning& bin) {
	return TopLeftRectangle((ImageRectangle)rectangle * bin,
		rectangle.within() * bin);
}

} // namespace image
} // namespace astro
