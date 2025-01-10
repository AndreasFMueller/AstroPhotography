/*
 * abadaper.h
 *
 * (c) 2024 Prof Dr Andreas Müller
 */
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroAdapter.h>
#include <deque>

namespace astro {
namespace adapter {

/**
 * \brief A template that maps various subframes of an image
 *
 * This adapter accepts pairs of rectangles of the source and target
 * image. When a pixel value is requested, it searches for the first
 * rectangle containing the pixel and returns the corresponding pixel
 * from the source image.
 */
template<typename Pixel>
class WindowsAdapter : public ConstImageAdapter<Pixel> {
typedef std::pair<ImageRectangle, WindowAdapter<Pixel> >	window_t;
	std::deque<window_t>	windows;
	const ConstImageAdapter<Pixel>&	image;

public:
	WindowsAdapter(const ConstImageAdapter<Pixel>& _image,
		const ImageSize& _size);
	void	add(const ImageRectangle& frame, const ImageRectangle& from);
	virtual Pixel	pixel(int x, int y) const;
};

/**
 * \brief Construct a Windows adapter image
 *
 * \param _image	the source image 
 * \param _targetsize	the size of the target image
 */
template<typename Pixel>
WindowsAdapter<Pixel>::WindowsAdapter(const ConstImageAdapter<Pixel>& _image,
		const ImageSize& _targetsize)
	: ConstImageAdapter<Pixel>(_targetsize), image(_image) {
}

/**
 * \brief Add a pair of rectangles to the WindowsAdapter
 *
 * \param targetrectangle		the target rectangle
 * \param sourcerectangle		the source
 */
template<typename Pixel>
void	WindowsAdapter<Pixel>::add(const ImageRectangle& targetrectangle,
		const ImageRectangle& sourcerectangle) {
	// XXX make sure the rectangles have the same size
	if (targetrectangle.size() != sourcerectangle.size()) {
		std::string	msg = astro::stringprintf();
	}
	window_t	newwindow(targetrectangle,
				WindowAdapter<Pixel>(image, sourcerectangle));
	windows.push_front(newwindow);
}

template<typename Pixel>
Pixel	WindowsAdapter<Pixel>::pixel(int x, int y) const {
	for (auto i = windows.begin(); i != windows.end(); i++) {
		if (i->first.contains(x, y)) {
			ImagePoint	p = i->first.subimage(x, y);
			return i->second.pixel(p);
		}
	}
	return Pixel();
}

} // namespace adapter
} // namespace astro
