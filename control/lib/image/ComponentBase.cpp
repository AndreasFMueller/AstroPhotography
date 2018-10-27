/*
 * ComponentBase.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <ConnectedComponent.h>
#include <AstroFilter.h>

namespace astro {
namespace image {

ComponentBase::ComponentBase(const ImageSize& size, const ImagePoint& point)
	: Image<unsigned char>(size), _point(point) {
	fill(0);
}

int	ComponentBase::grow() {
	unsigned int    width = getSize().width();
	unsigned int    height = getSize().height();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "growing in %dx%d image", width, height);

	// forward check
	int     counter_forward = 0;
	for (unsigned int y = 0; y < height; y++) {
		for (unsigned int x = 0; x < width; x++) {
			if (pixel(x, y) == 255) {
				continue;
			}
			if (255 == growpixel(x, y)) {
				writablepixel(x, y) = 255;
				counter_forward++;
			}
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "forward gave %d new points",
		counter_forward);

	// backward check
	int     counter_backward = 0;
	for (int y = height - 1; y >= 0; y--) {
		for (int x = width - 1; x >= 0; x--) {
			if (pixel(x, y) == 255) {
				continue;
			}
			if (255 == growpixel(x, y)) {
				writablepixel(x, y) = 255;
				counter_backward++;
			}
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "backward gave %d new points",
		counter_backward);

	// return the number of new pixels in this round
	return counter_forward + counter_backward;
}

int	ComponentBase::growpixel(unsigned int x, unsigned int y) {
	unsigned char   v = pixel(x, y);
	if (v == 0) {
		return 0;
	}
	if (v == 255) {
		return 255;
	}
	unsigned int    width = getSize().width();
	unsigned int    height = getSize().height();
	// left neighbor
	if (x > 0) {
		if (pixel(x - 1, y) == 255) {
			return 255;
		}
	}
	// bottom neighbor
	if (y > 0) {
		if (pixel(x, y - 1) == 255) {
			return 255;
		}
	}
	// right neighbor
	if (x < width - 1) {
		if (pixel(x + 1, y) == 255) {
			return 255;
		}
	}
	// top neighbor
	if (y < height - 1) {
		if (pixel(x, y + 1) == 255) {
			return 255;
		}
	}
	return v;
}

void	ComponentBase::process() {
	_npoints = 0;
	// special case: if the point is set, then we can grow the
	// connected component from it
	if (pixel(_point) > 0) {
		writablepixel(_point) = 255;
		int     newpixels = 0;
		do {
			newpixels = grow();
			_npoints += newpixels;
			debug(LOG_DEBUG, DEBUG_LOG, 0, "added %d new pixels",
				newpixels);
		} while (newpixels > 0);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no new pixels added");
	}

	// everything that is not marked so far has to be turned off
	debug(LOG_DEBUG, DEBUG_LOG, 0, "turn off pixels outside component");
	unsigned int	npixels = getSize().getPixels();
	for (unsigned int offset = 0; offset < npixels; offset++) {
		if ((*this)[offset] < 255) {
			(*this)[offset] = 0;
		}
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "component has %ld pixels", _npoints);
}

std::list<ImagePoint>	ComponentBase::points() const {
	std::list<ImagePoint>	result;
	int    width = getSize().width();
	int    height = getSize().height();
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			if (pixel(x, y) == 255) {
				ImagePoint	p(x, y);
				result.push_back(p);
			}
		}
	}
	return result;
}

double	ComponentBase::radius() {
	double	r = filter::MinRadius(points(), _center);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "component radius: %f, center %s", r,
		_center.toString().c_str());
	return r;
}

} // namespace image
} // namespace astro
