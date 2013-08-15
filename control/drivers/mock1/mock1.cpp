/*
 * mock1.cpp -- library structure
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroLoader.h>
#include <AstroCamera.h>
#include <includes.h>

using namespace astro::module;

namespace astro {
namespace module {
namespace mock1 {

static std::string	mock1_name("mock1");
static std::string	mock1_version(VERSION);

/**
 * \brief Descriptor of the mock1 module
 */
class Mock1Descriptor : public ModuleDescriptor {
public:
	Mock1Descriptor() { }
	virtual std::string	name() const {
		return mock1_name;
	}
	virtual std::string	version() const {
		return mock1_version;
	}
	virtual bool	hasDeviceLocator() const {
		return true;
	}
};

} // namespace mock1
} // namespace module
} // namespace astro

extern "C"
ModuleDescriptor	*getDescriptor() {
	return new astro::module::mock1::Mock1Descriptor();
}
