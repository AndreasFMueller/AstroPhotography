/*
 * Images.h -- image reference repository
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Images_h
#define _Images_h

#include <ObjWrapper.h>
#include <camera.hh>
#include <vector>

namespace astro {
namespace cli {

typedef ObjWrapper<Astro::Image>	ImageWrapper;

class Image_internals;

/**
 * \brief class to mediate access to ccd references by short name
 */
class Images {
	static Image_internals *internals;
public:
	Images();
	ImageWrapper	byname(const std::string& imageid);
	void	release(const std::string& imageid);
	void	assign(const std::string& imageid,
			const std::vector<std::string>& arguments);
};

} // namespace cli
} // namespace astro

#endif /* _Images_h */
