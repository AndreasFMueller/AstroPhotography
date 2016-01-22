/*
 * AsynchronousActionTest.cpp -- tests for the AsynchronousAction class
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include "../AsynchronousAction.h"
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <config.h>
#include <AstroDebug.h>

using namespace astro::guiding;

namespace astro {
namespace test {

class AsynchronousActionTest : public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	test();

	CPPUNIT_TEST_SUITE(AsynchronousActionTest);
	CPPUNIT_TEST(test);
	CPPUNIT_TEST_SUITE_END();
};

void	AsynchronousActionTest::setUp() {
}

void	AsynchronousActionTest::tearDown() {
}

class TestAction : public Action {
	int	_number;
	int	_repeats;
public:
	TestAction(int number, int repeats = 1)
		: _number(number), _repeats(repeats)  {
	}
	void	execute() {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "start %d/%d",
			_number, _repeats);
		while (_repeats--) {
			sleep(1);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "%d remain %d",
				_number, _repeats);
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "end %d", _number);
	}
};

void	AsynchronousActionTest::test() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "test() begin");
	AsynchronousAction	aa;

	int	counter = 0;
	while (counter++ < 10) {
		int	repeats = counter % 4;
		ActionPtr	a = ActionPtr(new TestAction(counter, repeats));
		bool	doesexecute = aa.execute(a);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%d does %sexecute", counter,
			(doesexecute) ? "" : "NOT ");
		sleep(3);
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "test() end");
}

CPPUNIT_TEST_SUITE_REGISTRATION(AsynchronousActionTest);

} // namespace test
} // namespace astro
