/*
 * AstroViewer.h -- Viewer
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroViewer_h
#define _AstroViewer_h

#include <AstroImage.h>
#include <stdint.h>

using namespace astro::image;

namespace astro {
namespace image {

class Viewer {
	ImagePtr	image;
	typedef	std::tr1::shared_ptr<uint32_t>	imagedataptr;
	imagedataptr	_imagedata;
	void	update();
public:
	Viewer(const std::string& filename);
	~Viewer();
	uint32_t	*imagedata() const;
	ImageSize	size() const;
};

} // namespace image
} // namespace astro

#endif /* _AstroViewer_h */
