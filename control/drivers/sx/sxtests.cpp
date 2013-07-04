/*
 * sxtests.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <debug.h>
#include <getopt.h>
#include <stdlib.h>

double	default_exposure = 0.01;
int	default_imagecount = 5;

int	main(int argc, char *argv[]) {
	int	c;
	while (EOF != (c = getopt(argc, argv, "de:n:")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'e':
			default_exposure = atof(optarg);
			break;
		case 'n':
			default_imagecount = atoi(optarg);
			break;
		}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "sx tests");
	CppUnit::TextUi::TestRunner	runner;
	CppUnit::TestFactoryRegistry	&registry
		= CppUnit::TestFactoryRegistry::getRegistry();
	runner.addTest(registry.makeTest());
	bool	wasSuccessful = runner.run("", false);
	return (wasSuccessful) ? 0 : 1;
}

