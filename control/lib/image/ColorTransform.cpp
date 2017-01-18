/*
 * ColorTransform.cpp
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroAdapter.h>

namespace astro {
namespace adapter {

static RGB<double>	parse_color(const std::string& s) {
	// split the string at commas
	std::vector<std::string>	components;
	split<std::vector<std::string> >(s, ",", components);
	if (3 != components.size()) {
		std::string	msg = stringprintf("not a color spec: '%s'",
			s.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	return RGB<double>(std::stod(components[0]),
			   std::stod(components[1]),
			   std::stod(components[2]));
}

void	ColorTransformBase::offsets(const std::string& o) {
	offsets(parse_color(o));
}

void	ColorTransformBase::scales(const std::string& s) {
	scales(parse_color(s));
}

#define	do_color(image, base, Pixel)					\
{									\
	Image<RGB<Pixel> >	*imagep					\
		= dynamic_cast<Image<RGB<Pixel> >*>(&*image);		\
	if (NULL != imagep) {						\
		return ColorTransformAdapter<Pixel>::color(*imagep, base);\
	}								\
}

ImagePtr	colortransform(ImagePtr image, const ColorTransformBase& colortransformbase) {
	do_color(image, colortransformbase, unsigned char)
	do_color(image, colortransformbase, unsigned short)
	do_color(image, colortransformbase, unsigned int)
	do_color(image, colortransformbase, unsigned long)
	do_color(image, colortransformbase, float)
	do_color(image, colortransformbase, double)
	throw std::runtime_error("cannot change color for this pixel type");
}


} // namespace adapter
} // namespace astro
