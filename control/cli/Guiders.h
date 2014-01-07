/*
 * Guiders.h -- repository of guider references
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Guiders_h
#define _Guiders_h

#include <AstroDebug.h>
#include <guider.hh>
#include <ObjWrapper.h>
#include <vector>

namespace astro {
namespace cli {

typedef ObjWrapper<Astro::Guider>	GuiderWrapper;

class Guider_internals;

/**
 * \brief class to mediate access to camera references by short name
 */
class Guiders {
	static Guider_internals	*internals;
public:
	Guiders();
	GuiderWrapper	byname(const std::string& cameraid);
	void	release(const std::string& cameraid);
	void	assign(const std::string& cameraid,
			const std::vector<std::string>& arguments);
};

} // namespace cli
} // namespace astro

#endif /* _Guiders_h */
