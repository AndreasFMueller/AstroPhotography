/*
 * AstroAdapter.h -- a collection of adapters
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroImage.h>

namespace astro {
namespace image {

/**
 * \brief Adapter for a subimage
 */
template<typename Pixel>
class WindowAdapter : public ConstImageAdapter<Pixel> {
	const Image<Pixel>&	image;
	ImageRectangle	frame;
public:
	WindowAdapter(const Image<Pixel>& image, const ImageRectangle& frame);
	virtual ImageSize	getSize() const;
	
	virtual const Pixel	pixel(unsigned int x, unsigned int y) const;
};

template<typename Pixel>
WindowAdapter<Pixel>::WindowAdapter(const Image<Pixel>& _image,
	const ImageRectangle& _frame) : image(_image), frame(_frame) {
}

template<typename Pixel>
ImageSize	WindowAdapter<Pixel>::getSize() const {
	return frame.size;
}

template<typename Pixel>
const Pixel	WindowAdapter<Pixel>::pixel(unsigned int x, unsigned int y) const {
	return	image.pixel(frame.origin.x + x, frame.origin.y + y);
}

/**
 * \brief Adapter to subimage with implied pixel type conversion
 */
template<typename Pixel, typename T>
class ConvertingWindowAdapter : public ConstImageAdapter<Pixel> {
	const Image<T>&	 image;
	ImageRectangle	frame;
public:
	ConvertingWindowAdapter(const Image<T>& image,
		const ImageRectangle& frame);
	virtual ImageSize	getSize() const;
	virtual const Pixel	pixel(unsigned int x, unsigned int y) const;
};

template<typename Pixel, typename T>
ConvertingWindowAdapter<Pixel, T>::ConvertingWindowAdapter(
	const Image<T>& _image, const ImageRectangle& _frame)
	: image(_image), frame(_frame) {
}

template<typename Pixel, typename T>
ImageSize	ConvertingWindowAdapter<Pixel, T>::getSize() const {
	return frame.size;
}

template<typename Pixel, typename T>
const Pixel	ConvertingWindowAdapter<Pixel, T>::pixel(unsigned int x, unsigned int y) const {
	const T&	t = image.pixel(frame.origin.x + x, frame.origin.y + y);
	// convert to Pixel type
	Pixel	p(t);
	return p;
}



} // namepsace image
} // namespace astro
