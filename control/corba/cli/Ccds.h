/*
 * Ccds.h -- ccd reference repository
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Ccds_h
#define _Ccds_h

#include <ObjWrapper.h>
#include <camera.hh>
#include <vector>

namespace astro {
namespace cli {

typedef ObjWrapper<Astro::Ccd>	CcdWrapper;

class Ccd_internals;

/**
 * \brief class to mediate access to ccd references by short name
 */
class Ccds {
	static Ccd_internals *internals;
public:
	Ccds();
	CcdWrapper	byname(const std::string& ccdid);
	void	release(const std::string& ccdid);
	void	assign(const std::string& ccdid,
			const std::vector<std::string>& arguments);
};

} // namespace cli
} // namespace astro

#endif /* _Ccds_h */
