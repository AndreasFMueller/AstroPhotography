/*
 * Focusers.h -- repository of focuser references
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Focusers_h
#define _Focusers_h

#include <AstroDebug.h>
#include <camera.hh>
#include <ObjWrapper.h>
#include <vector>

namespace astro {
namespace cli {

typedef ObjWrapper<Astro::Focuser>	FocuserWrapper;

class Focuser_internals;

/**
 * \brief class to mediate access to focuser references by short name
 */
class Focusers {
	static Focuser_internals	*internals;
public:
	Focusers();
	FocuserWrapper	byname(const std::string& focuserid);
	void	release(const std::string& focuserid);
	void	assign(const std::string& focuserid,
			const std::vector<std::string>& arguments);
};

} // namespace cli
} // namespace astro

#endif /* _Focusers_h */
