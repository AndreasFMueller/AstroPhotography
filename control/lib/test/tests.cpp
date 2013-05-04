/*
 * tests.cpp
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <includes.h>
#include <debug.h>

int	main(int argc, char *argv[]) {
	CppUnit::TextUi::TestRunner	runner;
	CppUnit::TestFactoryRegistry	&registry = CppUnit::TestFactoryRegistry::getRegistry();
	int	c;
	while (EOF != (c = getopt(argc, argv, "d")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		}
	runner.addTest(registry.makeTest());
	bool	wasSuccessful = runner.run("", false);
	return (wasSuccessful) ? 0 : 1;
}
