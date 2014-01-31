/*
 * tests.cpp
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <includes.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <AstroDebug.h>
#include <includes.h>
#include <OrbSingleton.h>

int	main(int argc, char *argv[]) {
#ifdef ENABLE_CORBA
	Astro::OrbSingleton	orb(argc, argv);
#endif /* ENABLE_CORBA */
	int	c;
	while (EOF != (c = getopt(argc, argv, "d")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
		}
	CppUnit::TextUi::TestRunner	runner;
	CppUnit::TestFactoryRegistry	&registry = CppUnit::TestFactoryRegistry::getRegistry();
	runner.addTest(registry.makeTest());
	bool	wasSuccessful = runner.run("", false);
	return (wasSuccessful) ? 0 : 1;
}
