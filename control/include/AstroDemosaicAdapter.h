/*
 * AstroDemosaicAdapter.h -- Adapter to demosaic an image
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _AstroDemosaicAdapter_h
#define _AstroDemosaicAdapter_h

#include <AstroImage.h>
#include <AstroAdapter.h>
#include <AstroDebug.h>

using namespace astro::image;

namespace astro {
namespace adapter {
namespace demosaic {

/**
 * \brief Base adapter for the other demosaicing adapters
 */
template<typename P>
class DemosaicAdapterBase : public ConstImageAdapter<P> {
public:
	const ConstImageAdapter<P>&	_image;
	MosaicType	_mosaic;
	int	_w;
	int	_h;
	int	_r;
	int	_b;
	int	_gr;
	int	_gb;
public:
	DemosaicAdapterBase(const ConstImageAdapter<P>& image,
		const MosaicType& mosaic)
		: ConstImageAdapter<P>(image.getSize()), _image(image),
		  _mosaic(mosaic) {
		_w = _image.getSize().width();
		_h = _image.getSize().height();
		{
			ImagePoint	p = _mosaic.red();
			_r = p.x() | (p.y() << 1);
		}
		{
			ImagePoint	p = _mosaic.blue();
			_b = p.x() | (p.y() << 1);
		}
		{
			ImagePoint	p = _mosaic.greenr();
			_gr = p.x() | (p.y() << 1);
		}
		{
			ImagePoint	p = _mosaic.greenb();
			_gb = p.x() | (p.y() << 1);
		}
	}
	virtual P	pixel(int x, int y) const { return _image.pixel(x, y); }
	virtual P	averageX(int x, int y) const;
	virtual P	averageCross(int x, int y) const;
	virtual P	averageH(int x, int y) const;
	virtual P	averageV(int x, int y) const;
};

template<typename P>
P	DemosaicAdapterBase<P>::averageX(int x, int y) const {
	int	counter = 0;
	double	p = 0;
	if (x > 0) {
		if (y > 0) {
			p += _image.pixel(x - 1, y - 1);
			counter++;
		}
		if (y + 1 < _h) {
			p += _image.pixel(x - 1, y + 1);
			counter++;
		}
	}
	if (x + 1 < _w) {
		if (y > 0) {
			p += _image.pixel(x + 1, y - 1);
			counter++;
		}
		if (y + 1 < _h) {
			p += _image.pixel(x + 1, y + 1);
			counter++;
		}
	}
	P	value = p / counter;
	return value;
}

template<typename P>
P	DemosaicAdapterBase<P>::averageCross(int x, int y) const {
	int	counter = 0;
	double	p = 0;
	if (x > 0) {
		p += _image.pixel(x - 1, y);
		counter++;
	}
	if (y > 0) {
		p += _image.pixel(x, y - 1);
		counter++;
	}
	if (x + 1 < _w) {
		p += _image.pixel(x + 1, y);
		counter++;
	}
	if (y + 1 < _h) {
		p += _image.pixel(x, y + 1);
		counter++;
	}
	P	value = p / counter;
	return value;
}

template<typename P>
P	DemosaicAdapterBase<P>::averageH(int x, int y) const {
	if (x == 0) {
		return _image.pixel(x + 1, y);
	}
	if (x + 1 == _w) {
		return _image.pixel(x - 1, y);
	}
	P	value = 0.5 * _image.pixel(x - 1, y) + 0.5 * _image.pixel(x + 1, y);
	return value;
}

template<typename P>
P	DemosaicAdapterBase<P>::averageV(int x, int y) const {
	if (y == 0) {
		return _image.pixel(x, y + 1);
	}
	if (y + 1 == _h) {
		return _image.pixel(x, y - 1);
	}
	P	value =  0.5 * _image.pixel(x, y - 1) + 0.5 * _image.pixel(x, y + 1);
	return value;
}

/**
 * \brief Adapter to demosaic the red channel of a bayer image
 */
template<typename P>
class DemosaicAdapterRed : public DemosaicAdapterBase<P> {
public:
	DemosaicAdapterRed(const ConstImageAdapter<P>& image,
		const MosaicType& mosaic)
		: DemosaicAdapterBase<P>(image, mosaic) {
		std::string	s = DemosaicAdapterBase<P>::_mosaic;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", s.c_str());
	}
	virtual P	pixel(int x, int y) const;
};

template<typename P>
P	DemosaicAdapterRed<P>::pixel(int x, int y) const {
	int	p = (x & 0x1) | ((y & 0x1) << 1);
	if (p == DemosaicAdapterBase<P>::_r) {
		return DemosaicAdapterBase<P>::_image.pixel(x, y);
	}
	if (p == DemosaicAdapterBase<P>::_b) {
		return DemosaicAdapterBase<P>::averageX(x, y);
	}
	if (p == DemosaicAdapterBase<P>::_gr) {
		return DemosaicAdapterBase<P>::averageH(x, y);
	}
	if (p == DemosaicAdapterBase<P>::_gb) {
		return DemosaicAdapterBase<P>::averageV(x, y);
	}
	return 0;
}

/**
 * \brief Adapter to demosaic the red channel of a bayer image
 */
template<typename P>
class DemosaicAdapterGreen : public DemosaicAdapterBase<P> {
public:
	DemosaicAdapterGreen(const ConstImageAdapter<P>& image,
		const MosaicType& mosaic)
		: DemosaicAdapterBase<P>(image, mosaic) {
	}
	virtual P	pixel(int x, int y) const;
};

template<typename P>
P	DemosaicAdapterGreen<P>::pixel(int x, int y) const {
	int	p = (x & 0x1) | ((y & 0x1) << 1);
	if ((p == DemosaicAdapterBase<P>::_gb) || (p == DemosaicAdapterBase<P>::_gr)) {
		return DemosaicAdapterBase<P>::_image.pixel(x, y);
	}
	return DemosaicAdapterBase<P>::averageCross(x, y);
}

/**
 * \brief Adapter to demosaic the red channel of a bayer image
 */
template<typename P>
class DemosaicAdapterBlue : public DemosaicAdapterBase<P> {
public:
	DemosaicAdapterBlue(const ConstImageAdapter<P>& image,
		const MosaicType& mosaic)
		: DemosaicAdapterBase<P>(image, mosaic) {
	}
	virtual P	pixel(int x, int y) const;
};

template<typename P>
P	DemosaicAdapterBlue<P>::pixel(int x, int y) const {
	int	p = (x & 0x1) | ((y & 0x1) << 1);
	if (p == DemosaicAdapterBase<P>::_b) {
		return DemosaicAdapterBase<P>::_image.pixel(x, y);
	}
	if (p == DemosaicAdapterBase<P>::_r) {
		return DemosaicAdapterBase<P>::averageX(x, y);
	}
	if (p == DemosaicAdapterBase<P>::_gb) {
		return DemosaicAdapterBase<P>::averageH(x, y);
	}
	if (p == DemosaicAdapterBase<P>::_gr) {
		return DemosaicAdapterBase<P>::averageV(x, y);
	}
	return 0;
}

/**
 * \brief Adapter to completely debayer a bayer image
 */
template<typename P>
class DemosaicAdapter : public ConstImageAdapter<RGB<P> > {
	const ConstImageAdapter<P>&	_image;
	const DemosaicAdapterRed<P>	_red;
	const DemosaicAdapterGreen<P>	_green;
	const DemosaicAdapterBlue<P>	_blue;
	bool	_nodebayer;
public:
	DemosaicAdapter(const ConstImageAdapter<P>& image,
		const MosaicType& mosaic)
		: ConstImageAdapter<RGB<P> >(image.getSize()), _image(image),
		  _red(image, mosaic), _green(image, mosaic),
		  _blue(image, mosaic) {
		_nodebayer = !mosaic;
	}
	virtual RGB<P>	pixel(int x, int y) const {
		if (_nodebayer) {
			return _image.pixel(x, y);
		}
		P	red = _red.pixel(x, y);
		P	green = _green.pixel(x, y);
		P	blue = _blue.pixel(x, y);
		return RGB<P>(red, green, blue);
	}
};

} // namespace demosaic
} // namespace adapter
} // namespace astro

#endif /* _AstroDemosaicAdapter_h */
