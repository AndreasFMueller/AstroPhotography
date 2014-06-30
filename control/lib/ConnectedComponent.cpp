/*
 * ConnectedComponent.cpp -- find the connected component of a point in an image
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <ConnectedComponent.h>
#include <AstroDebug.h>

namespace astro {
namespace image {

/**
 * \brief 
 */
unsigned char	ConnectedComponent::growpixel(Image<unsigned char>& image,
			unsigned int x, unsigned int y) const {
	unsigned char	v = image.pixel(x, y);
	if (v == 0) {
		return 0;
	}
	if (v == 255) {
		return 255;
	}
	unsigned int	width = image.getSize().width();
	unsigned int	height = image.getSize().height();
	// left neighbor
	if (x > 0) {
		if (image.pixel(x - 1, y) == 255) {
			return 255;
		}
	}
	// bottom neighbor
	if (y > 0) {
		if (image.pixel(x, y - 1) == 255) {
			return 255;
		}
	}
	// right neighbor
	if (x < width - 1) {
		if (image.pixel(x + 1, y) == 255) {
			return 255;
		}
	}
	// top neighbor
	if (y < height - 1) {
		if (image.pixel(x, y + 1) == 255) {
			return 255;
		}
	}
	return v;
}

/**
 * \brief Grow the connected component
 *
 * \param image	image to grow the connected component
 */
int	ConnectedComponent::grow(Image<unsigned char>& image) const {
	unsigned int	width = image.getSize().width();
	unsigned int	height = image.getSize().height();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "growing in %dx%d image", width, height);

	// forward check
	int	counter_forward = 0;
	for (unsigned int y = 0; y < height; y++) {
		for (unsigned int x = 0; x < width; x++) {
			if (image.pixel(x, y) == 255) {
				continue;
			}
			if (255 == growpixel(image, x, y)) {
				image.writablepixel(x, y) = 255;
				counter_forward++;
			}
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "forward gave %d new points",
		counter_forward);

	// backward check
	int	counter_backward = 0;
	for (int y = height - 1; y >= 0; y--) {
		for (int x = width - 1; x >= 0; x--) {
			if (image.pixel(x, y) == 255) {
				continue;
			}
			if (255 == growpixel(image, x, y)) {
				image.writablepixel(x, y) = 255;
				counter_backward++;
			}
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "backward gave %d new points",
		counter_backward);

	// return the number of new pixels in this round
	return counter_forward + counter_backward;
}

/**
 * \brief compute the connected component of the argument image
 */
ImagePtr	ConnectedComponent::operator()(const ImagePtr image) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "extraction connected component");

	// check that this is the image has the right type of pixel
	Image<unsigned char>	*imagep
		= dynamic_cast<Image<unsigned char> *>(&*image);
	if (NULL == imagep) {
		std::string	msg("connected component requires "
					"unsigned char pixel type");
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// create a new image
	Image<unsigned char>	*connected = new Image<unsigned char>(*imagep);

	// convert pixel values to 0/1
	unsigned int	npixels = connected->getSize().getPixels();
	unsigned int	counter = 0;
	for (int offset = 0; offset < npixels; offset++) {
		(*connected)[offset] = ((*connected)[offset] > 0) ? 1 : 0;
		counter += (*connected)[offset];
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "connected component has %u pixels",
		counter);

	// special case: if the point is set, then we can grow the
	// connected component from it
	if (connected->pixel(_point) > 0) {
		connected->writablepixel(_point) = 255;
		int	newpixels = 0;
		do {
			newpixels = grow(*connected);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "added %d new pixels",
				newpixels);
		} while (newpixels > 0);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no new pixels added");
	}

	// everything that is not marked so far has to be turned off
	debug(LOG_DEBUG, DEBUG_LOG, 0, "turn off pixels outside component");
	for (int offset = 0; offset < npixels; offset++) {
		if ((*connected)[offset] < 255) {
			(*connected)[offset] = 0;
		}
	}
	
	// encapsulate the result image into an ImagePtr
	return ImagePtr(connected);
}

} // namespace image
} // namespace astro
