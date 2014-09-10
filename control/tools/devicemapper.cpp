/*
 * devicemapper.cpp -- command to map devices to short names
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <typeinfo>
#include <stacktrace.h>

namespace astro {

int	main(int argc, char *argv[]) {
	return EXIT_SUCCESS;
}

} // namespace astro

int	main(int argc, char *argv[]) {
	signal(SIGSEGV, stderr_stacktrace);
	try {
		return astro::main(argc, argv);
	} catch (const std::exception& x) {
		std::cerr << "terminated by " << typeid(x).name() << ": ";
		std::cerr << x.what() << std::endl;
	} catch (...) {
		std::cerr << "terminated by unknown exception" << std::endl;
	}
	return EXIT_FAILURE;
}
