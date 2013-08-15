/*
 * mock2.cp
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroLoader.h>
#include <includes.h>

using namespace astro::module;

namespace astro {
namespace module {
namespace mock2 {

static std::string	mock2_name("mock2");
static std::string	mock2_version(VERSION);

class	Mock2Descriptor : public ModuleDescriptor {
public:
	Mock2Descriptor() { }
	~Mock2Descriptor() { }
	virtual std::string	name() const {
		return mock2_name;
	}
	virtual std::string	version() const {
		return mock2_version;
	}
};

} // namespace mock2
} // namespace module
} // namespace astro

extern "C"
ModuleDescriptor	*getDescriptor() {
	return new astro::module::mock2::Mock2Descriptor();
}
