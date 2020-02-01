/*
 * ConnectedComponentBase.cpp -- find the connected component of a point in an image
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroImage.h>
#include <AstroDebug.h>

namespace astro {
namespace image {

/**
 * \brief Construct a Connected component object
 *
 * \param point		the point starting the connected component
 */
ConnectedComponentBase::ConnectedComponentBase(const ImagePoint& point)
	: _point(point) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "connected component of point %s",
		_point.toString().c_str());
}

/**
 * \brief Construct a Connected component object
 *
 * This constructor also sets the region of interest which restricts 
 * where the connected component can live. All it's pixels must be
 * inside the region of interest.
 *
 * \param point		the point starting the connected component
 * \param roi		the region of interest
 */
ConnectedComponentBase::ConnectedComponentBase(const ImagePoint& point,
	const ImageRectangle& roi)
	: _point(point), _roi(roi) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "connected component within %s of %s",
		_roi.toString().c_str(), _point.toString().c_str());
}

/**
 * \brief Take the region of interest from the image if _roi not specified
 *
 * \param image		the image to take the roi from
 */
void	ConnectedComponentBase::setupRoi(const ImageRectangle& rectangle) {
	if (_roi.size() == ImageSize()) {
		_roi = rectangle;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "setting roi to %s",
			_roi.toString().c_str());
	}
}

/**
 * \brief Grow the connected component around a point
 *
 * \param image		the connected comopnent image to grow
 * \param x		the x coordinate of the point to grow
 * \param y		the y coordinate of the poitn to grow
 */
unsigned char	ConnectedComponentBase::growpixel(ImageAdapter<unsigned char>& image,
			int x, int y) const {
	unsigned char	v = image.pixel(x, y);
	if (v == 0) {
		return 0;
	}
	if (v == 255) {
		return 255;
	}
	// left neighbor
	if (x > _roi.xmin()) {
		if (image.pixel(x - 1, y) == 255) {
			return 255;
		}
	}
	// bottom neighbor
	if (y > _roi.ymin()) {
		if (image.pixel(x, y - 1) == 255) {
			return 255;
		}
	}
	// right neighbor
	if (x < _roi.xmax() - 1) {
		if (image.pixel(x + 1, y) == 255) {
			return 255;
		}
	}
	// top neighbor
	if (y < _roi.ymax() - 1) {
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
int	ConnectedComponentBase::grow(ImageAdapter<unsigned char>& image) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "growing in %s image in roi %s",
		image.getSize().toString().c_str(), _roi.toString().c_str());

	// get the region of interest boundaries
	int	xmin = _roi.xmin();
	int	ymin = _roi.ymin();
	int	xmax = _roi.xmax();
	int	ymax = _roi.ymax();

	// forward check
	int	counter_forward = 0;
	for (int y = ymin; y < ymax; y++) {
		for (int x = ymin; x < xmax; x++) {
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
	for (int y = ymax - 1; y >= ymin; y--) {
		for (int x = xmax - 1; x >= xmin; x--) {
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
 *
 * \param image		The image to compute the connected component
 */
WindowedImage<unsigned char>	*ConnectedComponentBase::component(
				const ConstImageAdapter<unsigned char>& image) {
	//int	w = image.getSize().width();
	//int	h = image.getSize().height();
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"extracting connected component from %s image at %s",
		image.getSize().toString().c_str(), _point.toString().c_str());

	// find out whether the roi is contained inside the image
	if (_roi.size() == ImageSize()) {
		_roi = ImageRectangle(image.getSize());
	} else {
		if (!image.getSize().bounds(_roi)) {
			std::string	msg = stringprintf(
				"%s is not contained in %s",
				_roi.toString().c_str(),
				image.getSize().toString().c_str());;
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "finding component inside roi %s",
		_roi.toString().c_str());

	// make sure the point is contained in the region of interest
	if (!_roi.contains(_point)) {
		std::string	msg = stringprintf("%s not in %s",
			_point.toString().c_str(), _roi.toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// create a new image
	WindowedImage<unsigned char>	*connected
		= new WindowedImage<unsigned char>(image.getSize(), _roi);
	// XXX connected->fill(0);

	// create the region of interest
	int	xmin = _roi.xmin();
	int	ymin = _roi.ymin();
	int	xmax = _roi.xmax();
	int	ymax = _roi.ymax();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "[%d,%d] x [%d,%d]",
		xmin, xmax, ymin, ymax);

	// convert pixel values to 0/1
	unsigned int	counter = 0;
	for (int x = xmin; x < xmax; x++) {
		for (int y = ymin; y < ymax; y++) {
			connected->writablepixel(x, y)
				= (image.pixel(x, y) > 0) ? 1 : 0;
			counter += connected->pixel(x, y);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%u candidate pixels", counter);

	// special case: if the point is set, then we can grow the
	// connected component from it
	int	componentpixels = 0;
	if (connected->ConstImageAdapter<unsigned char>::pixel(_point) > 0) {
		connected->ImageAdapter<unsigned char>::writablepixel(_point)
			= 255;
		componentpixels = 1;
		int	newpixels = 0;
		do {
			newpixels = grow(*connected);
			componentpixels += newpixels;
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"added %d new pixels (now %d)",
				newpixels, componentpixels);
		} while (newpixels > 0);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no new pixels added, total %d",
			componentpixels);
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "point %s is not accepted",
			_point.toString().c_str());
	}

	// everything that is not marked so far has to be turned off
	debug(LOG_DEBUG, DEBUG_LOG, 0, "turn off pixels outside component");
	for (int x = xmin; x < xmax; x++) {
		for (int y = ymin; y < ymax; y++) {
			if (connected->pixel(x, y) < 255)
				connected->writablepixel(x, y) = 0;
		}
	}
	
	// done
	return connected;
}

/**
 * \brief Count the points in a connected component
 *
 * \param connected	the image to count
 */
unsigned long	ConnectedComponentBase::count(
			const ConstImageAdapter<unsigned char>& connected) {
	int	w = connected.getSize().width();
	int	h = connected.getSize().height();
	unsigned int	counter = 0;
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			if (connected.pixel(x, y) == 255) {
				counter++;
			}
		}
	}
	return counter;
}

/**
 * \brief Count the points in a connected component
 *
 * \param connected	the image to count
 * \param roi		the region of interest to examine while counting
 */
unsigned long	ConnectedComponentBase::count(
			const ConstImageAdapter<unsigned char>& connected,
			const ImageRectangle& roi) {
	int	xmin = roi.xmin();
	int	ymin = roi.ymin();
	int	xmax = roi.xmax();
	int	ymax = roi.ymax();
	unsigned int	counter = 0;
	for (int x = xmin; x < xmax; x++) {
		for (int y = ymin; y < ymax; y++) {
			if (connected.pixel(x, y) == 255) {
				counter++;
			}
		}
	}
	return counter;
}

} // namespace image
} // namespace astro
