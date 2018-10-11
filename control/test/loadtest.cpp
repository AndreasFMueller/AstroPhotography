/*
 * loadtest.cpp -- test whether serial unloading really crashes the program
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroLoader.h>
#include <iostream>

using namespace astro::module;

void	moduletest(ModulePtr module) {
	ModuleDescriptorPtr	descriptor = module->getDescriptor();
	std::cout << descriptor->name() << ", " << descriptor->version()
		<< std::endl;
}

void	test() {
	ModuleRepositoryPtr	repository = getModuleRepository();
	ModulePtr	module1 = repository->getModule("mock1");
	module1->open();
#if 0
	moduletest(module1);
#else
	{
		ModuleDescriptorPtr	descriptor1 = module1->getDescriptor();
		std::cout << descriptor1->name() << ", "
			<< descriptor1->version() << std::endl;
	}
#endif
	module1->close();

	ModulePtr	module2 = repository->getModule("mock2");
	module2->open();
#if 0
	moduletest(module2);
#else
	{
		ModuleDescriptorPtr	descriptor2 = module2->getDescriptor();
		std::cout << descriptor2->name() << ", "
			<< descriptor2->version() << std::endl;
	}
#endif
	module2->close();
}

int	main(int /* argc */, char * /* argv */[]) {
	test();
}
