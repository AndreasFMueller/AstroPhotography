/*
 * AstroViewer.h -- Viewer
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroViewer_h
#define _AstroViewer_h

#include <AstroImage.h>
#include <AstroPixel.h>
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
	float	_backgroundluminance;
	RGB<float>	_backgroundcolor;
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

	float	backgroundluminance() const { return _backgroundluminance; }
	void	backgroundluminance(float backgroundluminance) {
		_backgroundluminance = backgroundluminance;
	}

	RGB<float>	backgroundcolor() const { return _backgroundcolor; }
	void	backgroundcolor(const RGB<float>& backgroundcolor) {
		_backgroundcolor = backgroundcolor;
	}

	float	gamma() const { return _gamma; }
	void	gamma(float gamma) { _gamma = gamma; }

	float	min() const { return _min; }
	void	min(float min) { _min = min; }

	float	max() const { return _max; }
	void	max(float max) { _max = max; }

	uint32_t	*imagedata() const;
	ImageSize	size() const;
};

} // namespace image
} // namespace astro

#endif /* _AstroViewer_h */
