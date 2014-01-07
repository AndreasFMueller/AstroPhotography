/*
 * exposurecommand.h -- tools to parse the exposure command
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _exposurecommand_h
#define _exposurecommand_h

#include <AstroCamera.h>

namespace astro {
namespace cli {

class ExposureParser {
	astro::camera::Exposure	_exposure;
public:
	const astro::camera::Exposure&	exposure() const { return _exposure; }
	void	exposure(const astro::camera::Exposure& e) { _exposure = e; }
public:
	ExposureParser();
	ExposureParser(const astro::camera::CcdInfo& info);
	ExposureParser(const astro::camera::Exposure& exposure);
	void	operator()(const std::string& valuepair);
	void	parse(const std::vector<std::string>& arguments,
			unsigned int offset = 0);
	astro::camera::Exposure	*operator->() { return &_exposure; }
};

} // namespace cli
} // namespace astro

#endif /* _exposurecommand_h */
