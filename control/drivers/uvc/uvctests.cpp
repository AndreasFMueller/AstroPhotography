/*
 * uvctests.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <debug.h>
#include <includes.h>

int	main(int argc, char *argv[]) {
	int	c;
	while (EOF != (c = getopt(argc, argv, "d")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		}
	
	CppUnit::TextUi::TestRunner	runner;
	CppUnit::TestFactoryRegistry	&registry
		= CppUnit::TestFactoryRegistry::getRegistry();
	runner.addTest(registry.makeTest());
	bool	wasSuccessful = runner.run("", false);
	return (wasSuccessful) ? 0 : 1;
}

