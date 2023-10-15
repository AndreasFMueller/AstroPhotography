/*
 * AstroWavelets.h -- adapters for the wavelet transform
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroWavelets_h
#define _AstroWavelets_h

#include <AstroImage.h>
#include <AstroAdapter.h>
#include <AstroDebug.h>
#include <AstroTypes.h>

using namespace astro::image;

namespace astro {
namespace adapter {


//////////////////////////////////////////////////////////////////////
// Haar Wavelet Adapter
//////////////////////////////////////////////////////////////////////
template<typename T>
class HaarWaveletXTransformAdapter : public ConstImageAdapter<T> {
	const ConstImageAdapter<T>&	_image;
	const int	w;
public:
	HaarWaveletXTransformAdapter(const ConstImageAdapter<T>& image)
		: ConstImageAdapter<T>(image.getSize()), _image(image),
		  w(image.getSize().width() / 2) {
	}
	virtual T	pixel(int x, int y) const {
		if (x >= w) {
			return (_image.pixel(2 * (x - w), y)
				- _image.pixel(2 * (x - w) + 1, y)) * 0.5;
		}
		return (_image.pixel(2 * x, y) + _image.pixel(2 * x + 1, y)) * 0.5;
	}
};

template<typename T>
class HaarWaveletXTransformInverseAdapter : public ConstImageAdapter<T> {
	const ConstImageAdapter<T>&	_image;
	const int	w;
public:
	HaarWaveletXTransformInverseAdapter(const ConstImageAdapter<T>& image)
		: ConstImageAdapter<T>(image.getSize()), _image(image),
		  w(image.getSize().width() / 2) {
	}
	virtual T	pixel(int x, int y) const {
		int	X = x / 2;
		if (0 == (x % 2)) {
			return _image.pixel(X, y) + _image.pixel(X + w, y);
		} else {
			return _image.pixel(X, y) - _image.pixel(X + w, y);
		}
	}
};

template<typename T>
class HaarWaveletYTransformAdapter : public ConstImageAdapter<T> {
	const ConstImageAdapter<T>&	_image;
	int 	h;
public:
	HaarWaveletYTransformAdapter(const ConstImageAdapter<T>& image)
		: ConstImageAdapter<T>(image.getSize()), _image(image),
		  h(image.getSize().height() / 2) {
	}
	virtual T	pixel(int x, int y) const {
		if (y >= h) {
			return (_image.pixel(x, 2 * (y - h))
				- _image.pixel(x, 2 * (y - h) + 1)) * 0.5;
		}
		return (_image.pixel(x, 2 * y + 1)
			+ _image.pixel(x, 2 * y)) * 0.5;
	}
};

template<typename T>
class HaarWaveletYTransformInverseAdapter : public ConstImageAdapter<T> {
	const ConstImageAdapter<T>&	_image;
	const int	h;
public:
	HaarWaveletYTransformInverseAdapter(const ConstImageAdapter<T>& image)
		: ConstImageAdapter<T>(image.getSize()), _image(image),
		  h(image.getSize().height() / 2) {
	}
	virtual T	pixel(int x, int y) const {
		int	Y = y / 2;
		if (0 == (y % 2)) { 
			return _image.pixel(x, Y) + _image.pixel(x, Y + h);
		} else {
			return _image.pixel(x, Y) - _image.pixel(x, Y + h);
		}
	}
};

template<typename T>
class HaarWaveletTransformAdapter : public ConstImageAdapter<T> {
	const HaarWaveletXTransformAdapter<T>	_xadapter;
	const HaarWaveletYTransformAdapter<T>	_yadapter;
public:
	HaarWaveletTransformAdapter(const ConstImageAdapter<T>& image)
		: ConstImageAdapter<T>(image.getSize()), _xadapter(image),
		  _yadapter(_xadapter) {
	}
	virtual T	pixel(int x, int y) const {
		return _yadapter.pixel(x, y);
	}
};

template<typename T>
class HaarWaveletTransformInverseAdapter : public ConstImageAdapter<T> {
	const HaarWaveletYTransformInverseAdapter<T>	_yadapter;
	const HaarWaveletXTransformInverseAdapter<T>	_xadapter;
public:
	HaarWaveletTransformInverseAdapter(const ConstImageAdapter<T>& image)
		: ConstImageAdapter<T>(image.getSize()), _yadapter(image),
		  _xadapter(_yadapter) {
	}
	virtual T	pixel(int x, int y) const {
		return _xadapter.pixel(x, y);
	}
};

template<typename T>
ImagePtr	haarwavelettransform(ConstImageAdapter<T>& image, bool inv) {
	if (inv) {
		HaarWaveletTransformInverseAdapter<T>	wt(image);
		return ImagePtr(new Image<T>(wt));
	} else {
		HaarWaveletTransformAdapter<T>	wt(image);
		return ImagePtr(new Image<T>(wt));
	}
	return ImagePtr();
}

ImagePtr	haarwavelettransform(ImagePtr image, bool inverse);

} // namespace adapter
} // namespace astro

#endif /* _AstroWavelets_h */

