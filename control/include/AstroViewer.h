/*
 * AstroViewer.h -- Viewer
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroViewer_h
#define _AstroViewer_h

#include <AstroImage.h>
#include <AstroPixel.h>
#include <AstroBackground.h>
#include <stdint.h>

using namespace astro::image;

namespace astro {
namespace image {

class Viewer {
	ImagePtr	image;
	typedef	std::tr1::shared_ptr<uint32_t>	imagedataptr;
	imagedataptr	_imagedata;
	void	update();

	RGB<float>	_colorcorrection;

	Background<float>	_background;
	bool	_backgroundsubtract;

	float	_gamma;
	float	_min;
	float	_max;
public:
	Viewer(const std::string& filename);
	~Viewer();

	RGB<float>	colorcorrection() const { return _colorcorrection; }
	void	colorcorrection(const RGB<float>& colorcorrection) {
		_colorcorrection = colorcorrection;
	}

	const Background<float>&	background() const {
		return _background;
	}
	bool	backgroundsubtract() const { return _backgroundsubtract; }
	void	backgroundsubtract(bool backgroundsubtract) {
		_backgroundsubtract = backgroundsubtract;
	}

	float	gamma() const { return _gamma; }
	void	gamma(float gamma) { _gamma = gamma; }

	float	min() const { return _min; }
	void	min(float min) { _min = min; }

	float	max() const { return _max; }
	void	max(float max) { _max = max; }

	void	writeimage(const std::string& filename);

	uint32_t	*imagedata() const;
	ImageSize	size() const;
};

} // namespace image
} // namespace astro

#endif /* _AstroViewer_h */
