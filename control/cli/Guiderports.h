/*
 * Guiderports.h -- repository of camera references
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Guiderports_h
#define _Guiderports_h

#include <AstroDebug.h>
#include <camera.hh>
#include <ObjWrapper.h>
#include <vector>

namespace astro {
namespace cli {

typedef ObjWrapper<Astro::GuiderPort>	GuiderPortWrapper;

class GuiderPort_internals;

/**
 * \brief class to mediate access to guiderport references by short name
 */
class Guiderports {
	static GuiderPort_internals	*internals;
public:
	Guiderports();
	GuiderPortWrapper	byname(const std::string& guiderportid);
	void	release(const std::string& guiderportid);
	void	assign(const std::string& guiderportid,
			const std::vector<std::string>& arguments);
};

} // namespace cli
} // namespace astro

#endif /* _Guiderports_h */
