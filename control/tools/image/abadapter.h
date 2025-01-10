/*
 * abadaper.h
 *
 * (c) 2024 Prof Dr Andreas Müller
 */
#include <AstroDebug.h>
#include <AstroAdapter.h>

namespace astro {
namespace adapter {

template<typename Pixel>
class WindowsAdapter : public ConstImageAdapter<Pixel> {
typedef std::pair<ImageRectangle, WindowAdapter<Pixel> >	window_t;
	std::dequeue<window_t>	windows;
	const ConstImageAdapter<Pixel>&	image;

public:
	WindowsAdapter(const ConstImageAdapter<Pixel>& _image,
		const ImageRectangle& _frame);
	void	add(const ImageRectangle& frame, const ImageRectangle& from);
	virtual Pixel	pixel(int x, int y) const;
};

template<typename Pixel>
WindowsAdapter::WindowsAdapter(const ConstImageAdapter<Pixel>& _image)
	: image(_image) {
}

template<typename Pixel>
void	WindowsAdapter::add(const ImageRectangle& frame,
		const ImageRectangle& from) {
	window_t	newwindow(frame, WindowAdapter(image, from));
	windows.push_front(newwindow);
}

template<typename Pixel>
Pixel	WindowsAdapter::pixel(int x, int y) const {
	for (auto i = windows.begin(); i != windows.end(); i++) {
		if (i->first.contains(x, y)) {
			return i->second.pixel(x, y);
		}
	}
	return Pixel;
}

} // namespace adapter
} // namespace astro
