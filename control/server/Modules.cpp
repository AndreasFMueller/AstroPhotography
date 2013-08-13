/*
 * Modules.cpp -- Modules servant implementation
 *
 * (c) Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <../idl/device.hh>
#include "Modules.h"
#include <AstroLoader.h>

namespace Astro {

/**
 * \brief Get the number of available modules.
 */
::CORBA::Long	Modules_impl::numberOfModules() {
	return repository.numberOfModules();
}

/**
 * \brief Get the list of module names.
 */
Astro::Modules::ModuleNameSequence*	Modules_impl::getModuleNames() {
	std::vector<std::string>	modules = repository.moduleNames();
	// convert the vector into a ModuleSequence
	Astro::Modules::ModuleNameSequence	*result
		= new Astro::Modules::ModuleNameSequence();
	result->length(modules.size());
	std::vector<std::string>::const_iterator	i;
	int	j = 0;
	for (i = modules.begin(); i != modules.end(); i++) {
		(*result)[j++] = ::CORBA::string_dup(i->c_str());
	}
	return result;
}

} // namespace Astro
