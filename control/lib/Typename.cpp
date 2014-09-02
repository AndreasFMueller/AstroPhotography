/*
 * Typename.cpp -- mixin class for type information
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <typeinfo>
#include <AstroFormat.h>

namespace astro {

/**
 * \brief Retrieve the type of a class
 */
std::string	Typename::type_name() const {
	try {
		return std::string(typeid(*this).name());
	} catch (std::bad_typeid& x) {
		return stringprintf("(bad type[%s])", x.what());
	}
}

} // namespace astro
