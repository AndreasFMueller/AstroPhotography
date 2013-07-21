/*
 * AstroGuiding.h -- 
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroStar_h
#define _AstroStar_h

namespace astro {
namespace guiding {

template<typename Pixel>
class StarDetector {
	const ConstImageIterator<Pixel>&	image;
public:
	StarDetecter(const ConstImageIterator<Pixel>& _image);
	Point	operator()(const ImageRectangle& rectangle) const;
}; 


template<typename Pixel>
StarDetector<Pixel>	StarDetecter(const ConstImageIterator<Pixel>& _image) : image(_image) {
}

template<typename Pixel>
Point	StarDetector<Pixel>::operator()(const ImageRectangle& rectangle) const {
	WindowAdapter<Pixel>	adapter(image, rectangle);
	ImageSize	size = adapter.getImage();
	unsigned	maxx = -1, maxy = -1;
	double	maxvalue = 0;
	for (unsigned int x = 0; x < size.width; x++) {
		for (unsigned int y = 0; y < size.height; y++) {
			double	value = luminance(adapter.pixel(x, y));
			if (value > maxvalue) {
				maxx = x; maxy = y; maxvalue = value;
			}
		}
	}

	double	xsum = 0, ysum = 0, weightsum = 0;
	for (unsigned int x = maxx - k; x <= maxx + k; x++) {
		for (unsigned int y = maxy - k; y <= maxy + k; y++) {
			double	value = luminance(adapter.pixel(x, y));
			weightsum += value;
			xsum += x * value;
			ysum += y * value;
			weightsum += value;
		}
	}
	return Point(xsum / weightsum, ysum / weightsum);
}

} // namespace guiding
} // namespace astro


#endif /* _AstroStar_h */
