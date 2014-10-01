/*
 * tests.cpp -- driver for CPP Unit tests
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <includes.h>
#include <AstroDebug.h>
#include <cstdlib>
#include <AstroUtils.h>

int	main(int argc, char *argv[]) {
	// install the segmentation fault handler
	signal(SIGSEGV, stderr_stacktrace);

	CppUnit::TextUi::TestRunner	runner;
	CppUnit::TestFactoryRegistry	&registry
		= CppUnit::TestFactoryRegistry::getRegistry();
	int	c;
	debugtimeprecision = 3;
	debugthreads = 1;
	while (EOF != (c = getopt(argc, argv, "d")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		}
	// remove the test database
	unlink("testdb.db");

	// perform the tests
	runner.addTest(registry.makeTest());

	// decide what return code to return
	bool	wasSuccessful = runner.run("", false);
	return (wasSuccessful) ? EXIT_SUCCESS : EXIT_FAILURE;
}
