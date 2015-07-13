/*
 * nicetests.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <includes.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <stdexcept>
#include <CommunicatorSingleton.h>

using namespace astro;

int	main(int argc, char *argv[]) {
	snowstar::CommunicatorSingleton	cs(argc, argv);
	int	c;
	debugthreads = true;
	while (EOF != (c = getopt(argc, argv, "de:n:t:p:")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "nice/ICE tests");
	CppUnit::TextUi::TestRunner	runner;
	CppUnit::TestFactoryRegistry	&registry
		= CppUnit::TestFactoryRegistry::getRegistry();
	runner.addTest(registry.makeTest());
	bool	wasSuccessful = runner.run("", false);
	return (wasSuccessful) ? 0 : 1;
}

